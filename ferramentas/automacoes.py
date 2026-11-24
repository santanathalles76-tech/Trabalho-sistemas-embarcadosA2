"""Conteúdo de cada automação usado para montar o artigo (main.tex)."""

AUTOMACOES = [
  dict(
    pasta="a1-controle-acesso", link="linkA", titulo="Controle de acesso por senha",
    problema=r"""A porta da área administrativa, onde ficam documentos de clientes e o arquivo financeiro, depende de chave física. Não há controle de quem entra e nada impede que alguém tente adivinhar uma senha numérica por tentativa e erro. A automação substitui a chave por uma trava eletrônica acionada por senha, com bloqueio temporário depois de erros seguidos.""",
    componentes=[(r"Teclado matricial de membrana 4x4", r"1"), (r"Servo motor (trava da porta)", r"1"), (r"Display LCD 16x2 com módulo I2C", r"1"), (r"LED verde e LED vermelho, com resistor de 220 $\Omega$", r"2"), (r"Buzzer passivo", r"1")],
    ligacoes=[(r"D9, D8, D7, D6", r"Linhas R1 a R4 do teclado"), (r"D5, D4, D3, D2", r"Colunas C1 a C4 do teclado"), (r"D10", r"Sinal (PWM) do servo"), (r"D11", r"Buzzer"), (r"D12", r"LED verde"), (r"D13", r"LED vermelho"), (r"A4 / A5", r"SDA / SCL do LCD")],
    logica=r"""O comportamento é uma máquina de três estados: \textsc{Aguardando}, \textsc{Liberado} e \textsc{Bloqueado}. Os dígitos ficam num vetor de até seis posições e aparecem mascarados no visor. A tecla \texttt{\#} confirma e a tecla \texttt{*} apaga. Toda a temporização usa \texttt{millis()}, sem travar o laço principal.""",
    pseudo=r"""estado <- AGUARDANDO; erros <- 0
repita para sempre:
  tecla <- ler teclado
  se estado = AGUARDANDO e tecla existe:
    se tecla é dígito e tamanho < 6: guardar dígito; mostrar '*'
    se tecla = '*': apagar senha digitada
    se tecla = '#':
      se senha = 4729: erros <- 0; abrir trava; LED verde; estado <- LIBERADO
      senão: erros <- erros + 1
             se erros >= 3: LED vermelho; som grave; estado <- BLOQUEADO
             senão: mostrar "Senha incorreta" e tentativas restantes
  se estado = LIBERADO e passaram 5 s: fechar trava; estado <- AGUARDANDO
  se estado = BLOQUEADO: ignorar teclas; mostrar contagem regressiva
    se passaram 15 s: erros <- 0; estado <- AGUARDANDO""",
    testes=[(r"Ligar o circuito", r"LCD ``Area restrita''; servo em 0$^\circ$; LEDs apagados"),
            (r"Digitar 4, 7", r"LCD ``Senha: **''"),
            (r"Tecla *", r"Senha apagada, LCD ``Senha:''"),
            (r"Digitar 4729 e \#", r"Servo em 90$^\circ$, LED verde, ``Acesso liberado''"),
            (r"Aguardar 5 s", r"Servo volta a 0$^\circ$, LED verde apaga"),
            (r"Digitar 1111 e \#", r"``Senha incorreta / Restam 2 tent.'', LED vermelho por 1,5 s"),
            (r"Errar mais duas vezes", r"``ACESSO BLOQUEADO'' com contagem de 15 s, LED vermelho"),
            (r"Digitar 4729 e \# durante o bloqueio", r"Porta continua travada"),
            (r"Aguardar o fim do bloqueio e digitar 4729 e \#", r"Trava abre normalmente"),
            (r"Digitar 10 dígitos seguidos", r"Só os 6 primeiros são aceitos")],
    captura="Teclado bloqueado depois de três senhas erradas"),

  dict(
    pasta="a2-sala-reuniao", link="linkB", titulo="Sala de reunião com aviso de ocupação",
    problema=r"""Sem sinalização, as pessoas abrem a porta da sala de reunião para conferir se ela está livre e interrompem conversas. Além disso, salas reservadas ficam vazias quando ninguém aparece (\emph{no-show}) e bloqueiam a agenda. A automação mostra na porta se a sala está livre, reservada ou ocupada e cancela sozinha a reserva que não foi usada.""",
    componentes=[(r"Sensor de presença PIR", r"1"), (r"Botão ``Reservar / Cancelar''", r"1"), (r"LEDs verde, amarelo e vermelho, com resistor de 220 $\Omega$", r"3"), (r"Display LCD 16x2 com módulo I2C", r"1")],
    ligacoes=[(r"D2", r"Saída (OUT) do PIR"), (r"D3", r"Botão, com pull-up interno"), (r"D9 / D10 / D11", r"LEDs verde / amarelo / vermelho"), (r"A4 / A5", r"SDA / SCL do LCD")],
    logica=r"""Máquina de três estados (\textsc{Livre}, \textsc{Reservada} e \textsc{Ocupada}). O botão é lido com \emph{debounce} de 50 ms e detecção de borda, de modo que cada aperto conta uma vez só. Os tempos foram reduzidos para a simulação: 20 s de tolerância para o check-in e 15 s sem movimento para liberar a sala (na prática, 10 e 5 minutos).""",
    pseudo=r"""estado <- LIVRE
repita para sempre:
  movimento <- PIR em nível alto; se movimento: últimoMovimento <- agora
  conforme estado:
    LIVRE:     se movimento: estado <- OCUPADA
               senão se botão apertado: estado <- RESERVADA; início <- agora
    RESERVADA: se movimento: estado <- OCUPADA          (check-in)
               se botão apertado: estado <- LIVRE        (cancelamento)
               se agora - início >= 20 s: estado <- LIVRE (no-show)
               mostrar segundos restantes no LCD
    OCUPADA:   se agora - últimoMovimento >= 15 s: estado <- LIVRE
  acender somente o LED do estado atual""",
    testes=[(r"Ligar o circuito", r"LED verde, LCD ``LIVRE''"),
            (r"Apertar o botão", r"LED amarelo, LCD ``RESERVADA 20s'' em contagem"),
            (r"Aguardar 20 s sem movimento", r"Reserva cancelada, volta a ``LIVRE''"),
            (r"Reservar e apertar o botão de novo", r"Reserva cancelada, ``LIVRE''"),
            (r"Reservar e simular movimento no PIR", r"Check-in: LED vermelho, ``OCUPADA''"),
            (r"Ficar 14 s sem movimento", r"Continua ``OCUPADA''"),
            (r"Novo movimento e mais 14 s", r"Continua ``OCUPADA'' (contagem reiniciada)"),
            (r"Completar 15 s sem movimento", r"Volta a ``LIVRE'', LED verde"),
            (r"Movimento com a sala livre", r"``OCUPADA'' direto, sem reserva")],
    captura="Sala reservada aguardando o check-in"),

  dict(
    pasta="a3-ar-condicionado", link="linkC", titulo="Ar-condicionado por ocupação e temperatura",
    problema=r"""O ar-condicionado das salas costuma ficar ligado com a sala vazia, ou liga e desliga sem parar perto da temperatura desejada, o que gasta energia e desgasta o compressor. A automação só refrigera quando há pessoas na sala e usa histerese e um intervalo mínimo entre acionamentos para proteger o equipamento. A temperatura desejada parte de 22 $^\circ$C, dentro da faixa de conforto de verão da ABNT NBR 16401-2 \cite{abnt16401}.""",
    componentes=[(r"Termistor NTC (módulo)", r"1"), (r"Potenciômetro (temperatura desejada)", r"1"), (r"Sensor de presença PIR", r"1"), (r"Módulo relé (compressor), com LED azul na carga", r"1"), (r"Display LCD 16x2 com módulo I2C", r"1")],
    ligacoes=[(r"A0", r"Saída do NTC"), (r"A1", r"Cursor do potenciômetro"), (r"D2", r"Saída (OUT) do PIR"), (r"D7", r"Entrada (IN) do relé; contato NA aciona o LED da carga"), (r"A4 / A5", r"SDA / SCL do LCD")],
    logica=r"""A temperatura vem da equação Beta do termistor ($\beta = 3950$). O potenciômetro define a temperatura desejada $T_d$ entre 18 e 28 $^\circ$C, em passos de 0,5 $^\circ$C. A sala é considerada ocupada se houve movimento nos últimos 20 s. O compressor liga quando $T \geq T_d + 1$ e desliga quando $T \leq T_d - 1$ ou quando a sala fica vazia. Entre duas mudanças do relé há pelo menos 5 s (na prática, 3 minutos).""",
    pseudo=r"""repita para sempre:
  se PIR em nível alto: últimoMovimento <- agora
  ocupada <- (agora - últimoMovimento < 20 s)
  T  <- 1 / (ln(1 / (1023 / leituraNTC - 1)) / 3950 + 1 / 298,15) - 273,15
  Td <- 18 + 10 * leituraPot / 1023   (arredondada a 0,5)
  deveLigar <- estado atual do compressor
  se não ocupada: deveLigar <- falso
  senão se T >= Td + 1: deveLigar <- verdadeiro
  senão se T <= Td - 1: deveLigar <- falso
  se deveLigar <> compressor e agora - últimaMudança >= 5 s:
    acionar relé; últimaMudança <- agora
  a cada 0,5 s: mostrar T, Td, ocupação e estado no LCD""",
    testes=[(r"NTC em 26 $^\circ$C, potenciômetro em 22 $^\circ$C, sem movimento", r"Relé desligado (sala vazia)"),
            (r"Simular movimento no PIR", r"Relé liga, LCD ``Ocupada AR LIG''"),
            (r"Baixar o NTC para 22,5 $^\circ$C", r"Continua ligado (dentro da histerese)"),
            (r"Baixar o NTC para 20,8 $^\circ$C", r"Relé desliga"),
            (r"Subir o NTC para 23,5 $^\circ$C logo em seguida", r"Espera 5 s antes de religar"),
            (r"Girar o potenciômetro até 28 $^\circ$C", r"Relé desliga (23,5 $^\circ$C já é frio para o alvo)"),
            (r"Voltar o potenciômetro a 22 $^\circ$C", r"Relé liga"),
            (r"Parar o movimento por 20 s", r"Relé desliga, LCD ``Vazia AR DESL''")],
    captura="Compressor ligado com a sala ocupada"),

  dict(
    pasta="a4-sala-servidores", link="linkD", titulo="Monitoramento da sala de servidores",
    problema=r"""Se a climatização da sala de servidores falha, a temperatura sobe em poucos minutos e os equipamentos desligam ou se danificam, muitas vezes fora do expediente. A automação classifica o ambiente em três níveis, liga uma ventilação de emergência e dispara alarme sonoro. Os limites seguem a faixa recomendada pela ASHRAE para data centers, de 18 a 27 $^\circ$C \cite{ashrae2021}.""",
    componentes=[(r"Sensor DHT22 (temperatura e umidade)", r"1"), (r"Botão ``Silenciar''", r"1"), (r"LEDs verde, amarelo e vermelho, com resistor de 220 $\Omega$", r"3"), (r"Buzzer passivo", r"1"), (r"Módulo relé (ventilação de emergência), com LED na carga", r"1"), (r"Display LCD 16x2 com módulo I2C", r"1")],
    ligacoes=[(r"D2", r"Dados (SDA) do DHT22"), (r"D3", r"Botão, com pull-up interno"), (r"D7", r"Entrada (IN) do relé"), (r"D8", r"Buzzer"), (r"D9 / D10 / D11", r"LEDs verde / amarelo / vermelho"), (r"A4 / A5", r"SDA / SCL do LCD")],
    logica=r"""O DHT22 é lido a cada 2 s, o intervalo mínimo do sensor. O nível é \textsc{Normal} entre 18 e 27 $^\circ$C com umidade de 20 a 80\%; \textsc{Atenção} fora dessas faixas ou se o sensor não responder; e \textsc{Crítico} acima de 32 $^\circ$C. A ventilação liga no nível crítico e só desliga abaixo de 30 $^\circ$C. O botão silencia o alarme atual, e uma nova piora faz o buzzer voltar a tocar.""",
    pseudo=r"""a cada 2 s:
  T, UR <- ler DHT22
  se leitura inválida:           nível <- ATENÇÃO
  senão se T > 32:               nível <- CRÍTICO
  senão se T fora de 18..27:     nível <- ATENÇÃO
  senão se UR fora de 20..80:    nível <- ATENÇÃO
  senão:                         nível <- NORMAL
  se nível mudou e nível <> CRÍTICO: silenciado <- falso
  acender LED do nível
  se nível = CRÍTICO: ventilação <- ligada
  senão se T < 30: ventilação <- desligada
sempre:
  se botão apertado e nível = CRÍTICO: silenciado <- verdadeiro
  buzzer intermitente (0,5 s) se nível = CRÍTICO e não silenciado""",
    testes=[(r"DHT22 em 24 $^\circ$C e 45\%", r"NORMAL, LED verde, ventilação desligada"),
            (r"Temperatura 29 $^\circ$C", r"ATENCAO, LED amarelo, sem som"),
            (r"24 $^\circ$C e umidade 85\%", r"ATENCAO"),
            (r"Temperatura 33 $^\circ$C", r"CRITICO, LED vermelho, ventilação ligada, buzzer intermitente"),
            (r"Apertar ``Silenciar''", r"Buzzer para; ventilação continua ligada"),
            (r"Temperatura 31 $^\circ$C", r"ATENCAO, ventilação ainda ligada (histerese)"),
            (r"Temperatura 29,5 $^\circ$C", r"Ventilação desliga"),
            (r"Temperatura 34 $^\circ$C", r"Alarme volta a tocar"),
            (r"Remover o fio de dados do DHT22 e reiniciar", r"ATENCAO e ``Falha no sensor'' no LCD")],
    captura="Nível crítico com ventilação de emergência ligada"),

  dict(
    pasta="a5-controle-lotacao", link="linkE", titulo="Contador de pessoas e controle de lotação do auditório",
    problema=r"""Em treinamentos e eventos internos o auditório pode passar do limite de pessoas previsto no projeto de segurança contra incêndio, e, numa evacuação, ninguém sabe quantas pessoas estão lá dentro. A Lei 13.425/2017 reforça a responsabilidade do estabelecimento sobre a lotação \cite{lei13425}. A automação conta entradas e saídas e trava a catraca quando o auditório está lotado.""",
    componentes=[(r"Botão do sensor de entrada (pedido de passagem)", r"1"), (r"Botão do sensor de saída", r"1"), (r"Servo motor (catraca de entrada)", r"1"), (r"LEDs verde, amarelo e vermelho, com resistor de 220 $\Omega$", r"3"), (r"Buzzer passivo", r"1"), (r"Display LCD 16x2 com módulo I2C", r"1")],
    ligacoes=[(r"D2", r"Botão de entrada, com pull-up interno"), (r"D3", r"Botão de saída, com pull-up interno"), (r"D6", r"Sinal (PWM) do servo"), (r"D8", r"Buzzer"), (r"D9 / D10 / D11", r"LEDs verde / amarelo / vermelho"), (r"A4 / A5", r"SDA / SCL do LCD")],
    logica=r"""Os dois botões representam os sensores da catraca e usam uma estrutura comum de \emph{debounce}. A capacidade foi reduzida para 8 pessoas na simulação. A entrada só é liberada se houver vaga; a saída nunca deixa o contador negativo. O LED muda de verde para amarelo a partir de 80\% da capacidade e para vermelho quando lota.""",
    pseudo=r"""ocupação <- 0
repita para sempre:
  se botão de entrada apertado:
    se ocupação < 8: ocupação <- ocupação + 1; abrir catraca por 2 s; bipe curto
    senão: manter catraca travada; aviso sonoro de lotação
  se botão de saída apertado e ocupação > 0: ocupação <- ocupação - 1
  percentual <- ocupação * 100 / 8
  LED verde se percentual < 80; amarelo se >= 80 e não lotado; vermelho se lotado
  se a catraca está aberta há 2 s: travar catraca""",
    testes=[(r"Ligar o circuito", r"LCD ``Pessoas: 0/8'', LED verde"),
            (r"Apertar entrada", r"1/8, servo em 90$^\circ$ por 2 s e volta a 0$^\circ$"),
            (r"Completar 6 entradas", r"6/8 (75\%), LED verde"),
            (r"Sétima entrada", r"7/8, LED amarelo, ``Quase lotado''"),
            (r"Oitava entrada", r"8/8, LED vermelho, ``LOTADO''"),
            (r"Nona entrada", r"Negada: contador fica em 8, catraca travada, aviso sonoro"),
            (r"Apertar saída", r"7/8"),
            (r"Apertar saída 9 vezes", r"Contador para em 0 (nunca negativo)")],
    captura="Auditório lotado com a catraca travada"),

  dict(
    pasta="a6-iluminacao-natural", link="linkF", titulo="Iluminação com aproveitamento da luz natural",
    problema=r"""As luminárias das estações de trabalho costumam ficar em 100\% mesmo com o sol entrando pela janela. A técnica de \emph{daylight harvesting} mede a luz natural e regula as luminárias para completar só o que falta. A meta adotada é de 500 lux, valor que a ABNT NBR ISO/CIE 8995-1 indica para tarefas de escrita e leitura em escritórios \cite{abnt8995}.""",
    componentes=[(r"Módulo sensor de luminosidade LDR (saída analógica)", r"1"), (r"LED branco (luminária dimerizável), com resistor de 220 $\Omega$", r"1"), (r"Display LCD 16x2 com módulo I2C", r"1")],
    ligacoes=[(r"A0", r"Saída analógica (AO) do LDR"), (r"D9", r"LED da luminária (PWM)"), (r"A4 / A5", r"SDA / SCL do LCD")],
    logica=r"""A leitura do LDR é convertida em lux pela curva do componente ($\gamma = 0{,}7$; 50 k$\Omega$ a 10 lux). Uma média móvel exponencial (peso 0,2 para a leitura nova) evita que uma nuvem ou a sombra de uma pessoa mude a luz de repente. A potência é proporcional ao que falta para 500 lux, e uma faixa morta de 5 passos de PWM (cerca de 2\%) impede que a luminária oscile.""",
    pseudo=r"""a cada 100 ms:
  lux <- converter(leitura do LDR)
  luxSuave <- 0,2 * lux + 0,8 * luxSuave
  se luxSuave >= 500: potência <- 0
  senão: potência <- (500 - luxSuave) / 500 * 255
  se |potência - potênciaAtual| >= 5 ou chegou a 0 ou a 255:
    potênciaAtual <- potência; PWM da luminária <- potênciaAtual
  a cada 0,5 s: mostrar lux e percentual no LCD""",
    testes=[(r"LDR em 50 lux", r"Luminária em cerca de 90\%"),
            (r"LDR em 250 lux", r"Luminária em cerca de 50\%"),
            (r"LDR em 400 lux", r"Luminária em cerca de 20\%"),
            (r"Saltar de 50 para 1000 lux", r"Brilho reduz aos poucos, sem apagar de uma vez"),
            (r"Manter 1000 lux", r"Luminária apagada (meta atingida pela luz natural)"),
            (r"Variar de 300 para 304 lux", r"Luminária não muda (faixa morta)")],
    captura="Luminária complementando a luz natural"),

  dict(
    pasta="a7-vazamento-gas", link="linkG", titulo="Detecção de vazamento de gás na copa",
    problema=r"""A copa do escritório tem fogão a gás. Um vazamento à noite ou no fim de semana pode passar despercebido e provocar explosão ou incêndio. A automação detecta o gás, fecha a válvula, liga o exaustor e só volta ao normal quando alguém confere o local e rearma o sistema.""",
    componentes=[(r"Sensor de gás MQ-2 (saídas analógica e digital)", r"1"), (r"Botão ``Rearmar''", r"1"), (r"Servo motor (válvula de gás)", r"1"), (r"Módulo relé (exaustor), com LED na carga", r"1"), (r"LED vermelho, com resistor de 220 $\Omega$", r"1"), (r"Buzzer passivo", r"1")],
    ligacoes=[(r"A0", r"Saída analógica (AOUT) do MQ-2"), (r"D4", r"Saída digital (DOUT) do MQ-2"), (r"D3", r"Botão, com pull-up interno"), (r"D6", r"Sinal (PWM) do servo"), (r"D7", r"Entrada (IN) do relé"), (r"D8", r"Buzzer"), (r"D9", r"LED vermelho")],
    logica=r"""Como o simulador não informa a curva de conversão do MQ-2 para ppm, o sistema mede o ar limpo nos primeiros 3 s e usa essa média como linha de base. O alarme dispara se a leitura passar da linha de base mais 150 ou se a saída digital do módulo for a nível baixo, o que dá redundância. O alarme fica travado (\emph{latch}): o rearme só é aceito com o gás abaixo da linha de base mais 75.""",
    pseudo=r"""durante 3 s: somar leituras; linhaBase <- média
repita para sempre:
  leitura <- AOUT; digital <- (DOUT = baixo)
  conforme estado:
    MONITORANDO: se leitura > linhaBase + 150 ou digital:
                   fechar válvula; ligar exaustor; estado <- ALARME
    ALARME: sirene de dois tons e LED piscando a cada 0,3 s
            se botão apertado:
              se leitura <= linhaBase + 75 e não digital:
                abrir válvula; desligar exaustor e sirene; estado <- MONITORANDO
              senão: recusar rearme""",
    testes=[(r"Ligar com o MQ-2 em ar limpo", r"Calibração de 3 s; linha de base registrada no monitor serial"),
            (r"Subir o gás um pouco (abaixo do limiar)", r"Sem alarme, válvula aberta"),
            (r"Subir o gás acima do limiar", r"Válvula fecha (90$^\circ$), exaustor liga, sirene e LED piscando"),
            (r"Apertar ``Rearmar'' com gás alto", r"Rearme recusado"),
            (r"Baixar o gás para nível seguro, sem apertar", r"Alarme continua (latch)"),
            (r"Apertar ``Rearmar''", r"Válvula abre, exaustor e sirene desligam"),
            (r"Atributo \texttt{threshold} do MQ-2 em 1 V e reiniciar", r"Alarme pela saída digital (redundância)")],
    captura="Alarme de gás com válvula fechada e exaustor ligado"),

  dict(
    pasta="a8-reservatorio-agua", link="linkH", titulo="Nível do reservatório e bomba de recalque",
    problema=r"""A caixa d'água que abastece banheiros e copa pode esvaziar sem aviso, transbordar com a bomba ligada ou fazer a bomba trabalhar a seco quando o sensor falha. A automação mede o nível por ultrassom, controla a bomba com histerese e protege o sistema contra falhas do sensor.""",
    componentes=[(r"Sensor ultrassônico HC-SR04", r"1"), (r"Módulo relé (bomba de recalque), com LED na carga", r"1"), (r"LEDs da barra de nível, com resistor de 220 $\Omega$", r"4"), (r"Buzzer passivo", r"1"), (r"Display LCD 16x2 com módulo I2C", r"1")],
    ligacoes=[(r"D12 / D13", r"TRIG / ECHO do HC-SR04"), (r"D7", r"Entrada (IN) do relé"), (r"D8", r"Buzzer"), (r"D2, D3, D4, D5", r"Barra de LEDs (25, 50, 75 e 100\%)"), (r"A4 / A5", r"SDA / SCL do LCD")],
    logica=r"""A distância até a água é a mediana de três medidas, o que descarta um eco espúrio. Com o sensor na tampa, 20 cm correspondem a 100\% e 220 cm a 0\%. A bomba liga abaixo de 25\% e desliga ao chegar a 90\%; entre esses valores mantém o estado anterior. Abaixo de 10\% há bipe de reserva crítica. Sem eco, a bomba é bloqueada.""",
    pseudo=r"""a cada 0,5 s:
  d <- mediana de 3 medidas do HC-SR04 (tempo do eco / 58)
  se não houve eco: falha <- verdadeiro; desligar bomba; mostrar falha
  senão:
    nível <- (220 - d) * 100 / (220 - 20), limitado a 0..100
    se nível < 25: ligar bomba
    senão se nível >= 90: desligar bomba
  se falha ou nível < 10: bipe a cada 2 s
  acender os LEDs da barra até o nível; mostrar nível e bomba no LCD""",
    testes=[(r"Distância 150 cm", r"Nível 35\%, bomba desligada, 1 LED da barra"),
            (r"Distância 180 cm", r"Nível 20\%: bomba liga"),
            (r"Distância 100 cm", r"Nível 60\%: bomba continua ligada, 2 LEDs"),
            (r"Distância 38 cm", r"Nível 91\%: bomba desliga"),
            (r"Distância 100 cm de novo", r"Bomba continua desligada (histerese)"),
            (r"Distância 205 cm", r"Nível 8\%: bomba liga e bipe de reserva crítica"),
            (r"Remover o fio do ECHO e reiniciar", r"``FALHA NO SENSOR'', bomba bloqueada, bipe")],
    captura="Bomba ligada enchendo o reservatório"),

  dict(
    pasta="a9-painel-senhas", link="linkI", titulo="Painel de senhas da recepção com atendimento preferencial",
    problema=r"""Na recepção, visitantes e fornecedores formam fila sem ordem definida, e o atendimento preferencial, garantido pela Lei 10.048/2000 \cite{lei10048}, fica a critério de quem está no balcão. A automação emite senhas normais e preferenciais e decide a ordem de chamada sem deixar a fila normal parada.""",
    componentes=[(r"Botões ``Senha normal'', ``Senha preferencial'' e ``Chamar próximo''", r"3"), (r"Display LCD 16x2 com módulo I2C (painel)", r"1"), (r"Buzzer passivo", r"1"), (r"LED ``Chamando'', com resistor de 220 $\Omega$", r"1")],
    ligacoes=[(r"D2 / D3 / D4", r"Botões normal / preferencial / chamar, com pull-up interno"), (r"D8", r"Buzzer"), (r"D9", r"LED ``Chamando''"), (r"A4 / A5", r"SDA / SCL do LCD")],
    logica=r"""Cada fila é controlada por dois contadores, de senhas emitidas e de senhas chamadas; quem aguarda é a diferença entre eles. A fila preferencial tem prioridade, mas depois de duas preferenciais seguidas o sistema chama uma normal, se houver alguém esperando. Essa regra evita que a fila normal fique parada indefinidamente.""",
    pseudo=r"""repita para sempre:
  se botão normal: emitidasN <- emitidasN + 1  (senha N001, N002...)
  se botão preferencial: emitidasP <- emitidasP + 1  (senha P001, P002...)
  se botão chamar:
    se ninguém aguarda: mostrar "Fila vazia"
    senão se há preferencial e (seguidas < 2 ou não há normal):
      chamar próxima P; seguidas <- seguidas + 1
    senão: chamar próxima N; seguidas <- 0
    tocar aviso de dois tons; acender LED por 3 s
  mostrar última senha chamada e o tamanho das filas no LCD""",
    testes=[(r"Apertar ``Chamar'' com as filas vazias", r"``Fila vazia''; nada é chamado"),
            (r"Emitir 3 senhas normais e 3 preferenciais", r"LCD ``Fila N:3 P:3''"),
            (r"Apertar ``Chamar'' 6 vezes", r"Ordem P001, P002, N001, P003, N002, N003"),
            (r"Logo após uma chamada", r"LED aceso e senha no painel"),
            (r"Aguardar 3 s", r"LED apaga"),
            (r"Só preferenciais na fila", r"Chamadas em sequência, sem travar")],
    captura="Painel chamando a próxima senha"),
]
