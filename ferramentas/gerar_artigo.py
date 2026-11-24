"""Monta artigo/main.tex a partir do conteúdo das automações e do código-fonte."""
import os
from automacoes import AUTOMACOES

RAIZ = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SAIDA = os.path.join(RAIZ, "artigo", "main.tex")
ROMANOS = ["A", "B", "C", "D", "E", "F", "G", "H", "I"]

PREAMBULO = r"""\documentclass[12pt]{article}

\usepackage{sbc-template}
\usepackage{graphicx,url}
\usepackage[brazil]{babel}
\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc}
\usepackage{listings}
\usepackage{xcolor}
\usepackage{tabularx}
\usepackage{booktabs}
\usepackage{float}
\usepackage[hidelinks]{hyperref}

% Pasta de cada simulação no repositório do trabalho
\newcommand{\repositorio}{https://github.com/santanathalles76-tech/Trabalho-sistemas-embarcadosA2}
\newcommand{\simlink}[1]{\href{\repositorio/tree/main/wokwi/#1}{\texttt{wokwi/#1}}}

% Resultado de cada linha das tabelas de teste (conferido pelos testes automatizados).
\newcommand{\conf}{Conforme}

% Insere a captura de tela se o arquivo já foi enviado; se não, mostra um
% quadro indicando qual imagem falta (assim o PDF compila em qualquer etapa).
\newcommand{\captura}[3]{%
  \begin{figure}[!htb]
  \centering
  \IfFileExists{#1}{\includegraphics[width=.85\textwidth]{#1}}{%
    \fbox{\parbox[c][4.2cm][c]{.8\textwidth}{\centering\small\sffamily
      Inserir captura de tela: \texttt{#1}\\[4pt] #2}}}
  \caption{#2}
  \label{#3}
  \end{figure}}

% Código-fonte com acentos (o listings não lê UTF-8 sozinho)
\definecolor{comentario}{RGB}{90,110,90}
\definecolor{palavra}{RGB}{20,60,140}
\definecolor{texto}{RGB}{150,60,30}
\lstset{
  basicstyle=\ttfamily\scriptsize, breaklines=true, columns=fullflexible,
  keepspaces=true, showstringspaces=false, frame=single, rulecolor=\color{black!30},
  numbers=left, numberstyle=\tiny\color{black!50}, numbersep=6pt, xleftmargin=1.5em,
  commentstyle=\color{comentario}\itshape, keywordstyle=\color{palavra}\bfseries,
  stringstyle=\color{texto},
  literate={á}{{\'a}}1 {à}{{\`a}}1 {â}{{\^a}}1 {ã}{{\~a}}1 {é}{{\'e}}1 {ê}{{\^e}}1
           {í}{{\'i}}1 {ó}{{\'o}}1 {ô}{{\^o}}1 {õ}{{\~o}}1 {ú}{{\'u}}1 {ç}{{\c{c}}}1
           {Á}{{\'A}}1 {Ã}{{\~A}}1 {É}{{\'E}}1 {Í}{{\'I}}1 {Ó}{{\'O}}1 {Ç}{{\c{C}}}1
           {°}{{\textdegree}}1 {µ}{{$\mu$}}1 {Ω}{{$\Omega$}}1
}
\lstdefinestyle{pseudo}{language={}, numbers=none, xleftmargin=0pt,
  basicstyle=\ttfamily\footnotesize, frame=lines}

\sloppy

\title{Escritório Inteligente: nove automações com Arduino Uno\\
para simulação no Wokwi}

\author{Thalles da Silva Santana (matrícula 1240110579)}

\address{Ciência da Computação, Universidade Veiga de Almeida (UVA)\\
  Disciplina de Sistemas Embarcados, Prof. Caio Silva Azeredo\\
  Rio de Janeiro, RJ, Brasil}

\begin{document}

% ============================ CAPA ====================================
\begin{titlepage}
\centering\sffamily\fontsize{12}{14.5}\selectfont
\vspace*{1.2cm}
\includegraphics[width=3.2cm]{logo-uva.jpg}\par
\vspace{0.5cm}
{\bfseries UNIVERSIDADE VEIGA DE ALMEIDA\\ CIÊNCIA DA COMPUTAÇÃO}\\
SISTEMAS EMBARCADOS\par
\vspace{0.5cm}
Thalles da Silva Santana\\ 1240110579\par
\vspace{4.2cm}
{\fontsize{14}{17}\selectfont\bfseries ESCRITÓRIO INTELIGENTE:\\ NOVE AUTOMAÇÕES COM ARDUINO UNO\\ PARA SIMULAÇÃO NO WOKWI\par}
\vspace{0.9cm}
Trabalho opcional da A2 apresentado como artigo de\\ automação com sistemas embarcados\par
\vspace{1.3cm}
Orientador: Prof. Caio Silva Azeredo\par
\vfill
Rio de Janeiro/RJ\\ 2026\par
\vspace*{1.5cm}
\end{titlepage}
\setcounter{page}{1}

\maketitle

\begin{abstract}
This paper presents the simulated automation of a corporate office with nine embedded systems based on the Arduino Uno and designed for the Wokwi simulator: access control, meeting room signage, occupancy-based air conditioning, server room monitoring, people counting, daylight harvesting, gas leak detection, water tank control and a reception queue with priority service. Each one reads sensors or buttons, applies decision logic, often a finite state machine, and drives LEDs, buzzers, servos, relays or displays. The programs were compiled for the ATmega328P and checked by 84 automated logic tests.
\end{abstract}

\begin{resumo}
Este artigo apresenta a automação simulada de um escritório corporativo com nove sistemas embarcados baseados no Arduino Uno e projetados para o simulador Wokwi: controle de acesso, sinalização da sala de reunião, ar-condicionado por ocupação, monitoramento da sala de servidores, contagem de pessoas, aproveitamento da luz natural, detecção de gás, controle do reservatório e painel de senhas com atendimento preferencial. Cada uma lê sensores ou botões, aplica uma lógica de decisão, muitas vezes uma máquina de estados, e aciona LEDs, buzzers, servos, relés ou displays. Os programas foram compilados para o ATmega328P e verificados por 84 testes automatizados de lógica.
\end{resumo}

\noindent\textbf{Palavras-chave:} sistemas embarcados; Arduino; automação predial; escritório inteligente; Wokwi; máquina de estados.
"""

INTRO = r"""
\section{Introdução}

Escritórios concentram desperdícios e riscos que passam despercebidos na rotina: o ar-condicionado ligado em sala vazia, luminárias acesas com o sol entrando pela janela, salas de reunião reservadas e não usadas, portas controladas por chave física e equipamentos críticos sem monitoramento fora do expediente. Sistemas embarcados de baixo custo permitem tratar esses problemas com sensores, atuadores e regras de decisão simples, executadas localmente e em tempo real.

O ambiente escolhido para este trabalho é um \textbf{escritório corporativo} de médio porte, com recepção, área administrativa, salas de reunião, auditório para treinamentos, estações de trabalho, sala de servidores, copa e um reservatório de água próprio. O problema central é tornar esse ambiente mais seguro, econômico e organizado sem depender de supervisão humana constante.

O objetivo é projetar, simular e testar \textbf{nove automações} para esse escritório, cada uma com ao menos uma entrada, uma saída e uma lógica de decisão própria. Todas foram desenvolvidas em C++ para o Arduino Uno, com projetos prontos para o simulador Wokwi. A Seção~\ref{sec:referencial} apresenta os conceitos usados; a Seção~\ref{sec:metodologia}, as ferramentas e os componentes; a Seção~\ref{sec:automacoes} detalha as automações; a Seção~\ref{sec:resultados} discute os resultados e a integração entre elas; e a Seção~\ref{sec:conclusao} traz limitações e melhorias futuras. O código completo está nos apêndices.

\section{Referencial teórico}\label{sec:referencial}

\subsection{Sistemas embarcados}

Sistema embarcado é um sistema computacional dedicado a uma função específica, integrado ao equipamento que controla e, em geral, sujeito a restrições de custo, consumo, memória e tempo de resposta \cite{marwedel2021,wolf2017}. Diferente de um computador de uso geral, ele interage continuamente com o mundo físico: lê grandezas por meio de sensores, processa essas informações e age sobre o ambiente por meio de atuadores. Lee e Seshia \cite{lee2017} descrevem esses sistemas como ciberfísicos e destacam o uso de máquinas de estados finitos para modelar o comportamento reativo, abordagem adotada em várias automações deste trabalho.

\subsection{Sensores e atuadores}

Sensores convertem grandezas físicas em sinais elétricos. Os digitais, como o sensor de presença PIR e os botões, entregam dois níveis lógicos. Os analógicos, como o termistor NTC, o resistor dependente de luz (LDR), o potenciômetro e o sensor de gás MQ-2, entregam uma tensão proporcional à grandeza, lida pelo conversor analógico-digital (ADC) de 10 bits do microcontrolador (valores de 0 a 1023). Há ainda sensores com protocolo próprio, como o DHT22 (temperatura e umidade em um único fio) e o ultrassônico HC-SR04, que mede distância pelo tempo de ida e volta de um pulso sonoro.

Atuadores convertem sinais elétricos em ação: LEDs e displays informam, buzzers alertam, servos posicionam travas e válvulas, e módulos relé permitem que um sinal de 5 V comande cargas de potência, como compressores, bombas e exaustores. A modulação por largura de pulso (PWM) permite variar a potência média entregue a uma carga, recurso usado para dimerizar a iluminação.

\subsection{Plataforma: Arduino Uno e ATmega328P}

O Arduino Uno é uma placa de desenvolvimento baseada no microcontrolador ATmega328P, de 8 bits e arquitetura AVR, com 32 KB de memória flash, 2 KB de SRAM e relógio de 16 MHz \cite{microchip2018,arduino2024}. A placa oferece 14 pinos digitais, 6 deles com PWM, 6 entradas analógicas e comunicação serial, I2C e SPI. A programação é feita em C++ com as bibliotecas do ecossistema Arduino \cite{mcroberts2018,banzi2014}. Um ponto importante do ATmega328P é que os temporizadores são compartilhados: a função \texttt{tone()} usa o \emph{Timer2} e a biblioteca \texttt{Servo} usa o \emph{Timer1}, o que desativa o PWM em alguns pinos e precisa ser considerado na escolha das ligações.

\section{Metodologia}\label{sec:metodologia}

\subsection{Simulador, linguagem e placa}

As automações foram projetadas para o \textbf{Wokwi} \cite{wokwi2024}, simulador on-line que executa o código compilado do Arduino e oferece sensores interativos: durante a simulação é possível clicar no PIR, ajustar a temperatura do NTC e do DHT22, a luminosidade do LDR, a concentração de gás do MQ-2 e a distância do HC-SR04. A linguagem é C++ (Arduino) e a placa é o \textbf{Arduino Uno}. Cada automação é um projeto independente no Wokwi, com o programa (\texttt{sketch.ino}), o circuito (\texttt{diagram.json}) e a lista de bibliotecas (\texttt{libraries.txt}).

Os tempos foram reduzidos para que os testes coubessem numa simulação, e cada programa indica o valor que seria usado na prática. Todos os programas usam \texttt{millis()} em vez de \texttt{delay()} para controlar o tempo, de modo que o laço principal continua lendo as entradas enquanto espera.

\subsection{Verificação}

A verificação foi feita em três etapas. Primeiro, cada programa foi compilado com o compilador \texttt{avr-gcc} e o núcleo oficial do Arduino para o ATmega328P, sem nenhum aviso de compilação; a Tabela~\ref{tab:memoria} mostra o uso de memória. Segundo, a lógica de decisão foi exercitada por \textbf{84 testes automatizados}, executados no computador com uma simulação mínima das funções do Arduino, que reproduzem as sequências de entrada descritas nas tabelas de teste da Seção~\ref{sec:automacoes}. Todos os testes passaram. Terceiro, os binários das Automações 2, 5 e 9 foram executados no avr8js, o emulador de código aberto do ATmega328P que serve de base ao Wokwi, com a sequência de botões e sensores aplicada aos pinos e a saída serial registrada, que coincidiu com a esperada.

\begin{table}[!htb]
\centering
\caption{Uso de memória de cada automação no ATmega328P}\label{tab:memoria}
\small
\begin{tabularx}{\textwidth}{Xrr}
\toprule
Automação & Flash (de 32 KB) & SRAM (de 2 KB) \\
\midrule
%MEMORIA%
\bottomrule
\end{tabularx}
\end{table}

\subsection{Componentes}

A Tabela~\ref{tab:componentes} resume os componentes usados. Todos estão disponíveis na biblioteca de peças do Wokwi. Os LEDs usam resistores de 220 $\Omega$ em série, e os botões são ligados entre o pino e o GND, com o resistor de \emph{pull-up} interno do microcontrolador ativado no código.

\begin{table}[!htb]
\centering
\caption{Componentes utilizados}\label{tab:componentes}
\small
\begin{tabularx}{\textwidth}{Xcc}
\toprule
Componente & Quantidade total & Automações \\
\midrule
Arduino Uno (ATmega328P) & 9 & todas \\
Display LCD 16x2 com módulo I2C (endereço 0x27) & 8 & 1, 2, 3, 4, 5, 6, 8, 9 \\
Teclado matricial de membrana 4x4 & 1 & 1 \\
Sensor de presença PIR & 2 & 2, 3 \\
Termistor NTC (módulo) & 1 & 3 \\
Potenciômetro & 1 & 3 \\
Sensor DHT22 (temperatura e umidade) & 1 & 4 \\
Módulo sensor LDR & 1 & 6 \\
Sensor de gás MQ-2 & 1 & 7 \\
Sensor ultrassônico HC-SR04 & 1 & 8 \\
Botão (\emph{push button}) & 8 & 2, 4, 5, 7, 9 \\
Servo motor & 3 & 1, 5, 7 \\
Módulo relé de 1 canal & 4 & 3, 4, 7, 8 \\
LEDs (com resistor de 220 $\Omega$) & 25 & todas \\
Buzzer passivo & 6 & 1, 4, 5, 7, 8, 9 \\
\bottomrule
\end{tabularx}
\end{table}

\subsection{Procedimento de teste}

Para cada automação, montou-se uma tabela de entrada e saída com os casos normais, os limites (por exemplo, o valor exato em que a bomba liga) e as situações de falha (sensor sem resposta, senha errada repetida, lotação esgotada). Cada caso foi reproduzido nos testes automatizados, que conferem o estado dos LEDs, o texto do display, a posição dos servos, os relés e o buzzer. Os mesmos casos podem ser repetidos no Wokwi, onde o monitor serial registra cada decisão do programa.

\section{Automações}\label{sec:automacoes}

As nove automações se distribuem pelo escritório conforme a Tabela~\ref{tab:mapa}.

\begin{table}[!htb]
\centering
\caption{Distribuição das automações no escritório}\label{tab:mapa}
\small
\begin{tabularx}{\textwidth}{clX}
\toprule
\# & Local & Automação \\
\midrule
1 & Área administrativa & Controle de acesso por senha \\
2 & Salas de reunião & Aviso de livre, reservada ou ocupada \\
3 & Salas de reunião e escritórios & Ar-condicionado por ocupação e temperatura \\
4 & Sala de servidores & Monitoramento de temperatura e umidade \\
5 & Auditório & Contador de pessoas e controle de lotação \\
6 & Estações de trabalho & Iluminação com aproveitamento da luz natural \\
7 & Copa & Detecção de vazamento de gás \\
8 & Cobertura & Nível do reservatório e bomba de recalque \\
9 & Recepção & Painel de senhas com atendimento preferencial \\
\bottomrule
\end{tabularx}
\end{table}
"""

RESTO = r"""
\section{Resultados e discussão}\label{sec:resultados}

As nove automações funcionaram como descrito: todos os casos das tabelas de teste produziram as saídas esperadas nos 84 testes automatizados de lógica, as execuções no emulador AVR coincidiram com o esperado e os programas compilaram para o ATmega328P sem avisos, usando no máximo 40\% da memória flash e 40\% da SRAM (Tabela~\ref{tab:memoria}). Há, portanto, folga para recursos futuros, como comunicação em rede.

\subsection{Dificuldades}

A primeira dificuldade foi a \textbf{distância entre o tempo real e o tempo de teste}. Esperar 5 minutos para uma sala ser liberada ou 3 minutos para proteger um compressor tornaria os testes inviáveis; por isso os tempos foram parametrizados em constantes no início de cada programa e reduzidos para a simulação.

A segunda foi a \textbf{calibração de sensores analógicos}. O MQ-2 do Wokwi não informa a curva de conversão da tensão para ppm, e o valor absoluto da leitura varia com o modelo e o ambiente. A solução foi medir a linha de base do ar limpo ao ligar o sistema e decidir por variação relativa, somando a saída digital do módulo como segunda fonte de alarme. Para o NTC e o LDR, usaram-se as equações físicas dos componentes, o que torna as leituras comparáveis com valores de norma, como os 500 lux da NBR ISO/CIE 8995-1.

A terceira foi \textbf{evitar oscilações}. Controles do tipo liga e desliga perto de um limite tendem a acionar o atuador repetidamente. Isso foi tratado com histerese (ar-condicionado, ventilação da sala de servidores e bomba), intervalo mínimo entre acionamentos (compressor), média móvel e faixa morta (iluminação) e \emph{debounce} em todos os botões.

Por fim, houve o cuidado com os \textbf{recursos compartilhados do microcontrolador}: os temporizadores usados por \texttt{tone()} e pela biblioteca \texttt{Servo} desativam o PWM em alguns pinos, e por isso a luminária da Automação~6, que depende de PWM, não divide a placa com um servo.

\subsection{Integração no ambiente}

Embora cada automação rode em uma placa própria, todas pertencem ao mesmo projeto e se complementam ao longo do dia de trabalho. Um visitante chega à recepção e recebe uma senha (Automação~9); o colaborador entra na área administrativa pela porta com senha (Automação~1); a reunião começa e a sala passa a ``ocupada'' (Automação~2), o que também mantém o ar-condicionado ativo (Automação~3), enquanto as estações de trabalho aproveitam a luz natural (Automação~6). À tarde, o treinamento no auditório respeita a lotação (Automação~5). Fora do expediente, os sistemas de segurança continuam atentos: sala de servidores (Automação~4), copa (Automação~7) e reservatório (Automação~8).

O sensor de presença é o ponto de integração mais evidente: a mesma informação de ocupação usada na sinalização da sala de reunião pode desligar o ar-condicionado e a iluminação quando a sala esvazia. Do mesmo modo, a contagem do auditório pode alimentar o ar-condicionado com a carga térmica real do ambiente.

\section{Conclusão}\label{sec:conclusao}

O trabalho mostrou que um único microcontrolador de 8 bits, com componentes de baixo custo, resolve problemas reais de um escritório corporativo quando a lógica de decisão é bem modelada. Máquinas de estados, histerese, filtros simples e detecção de falhas transformaram leituras de sensores em decisões seguras e previsíveis.

\textbf{Limitações.} Os resultados vêm de simulação: sensores reais têm ruído, deriva e tempos de aquecimento (o MQ-2 físico precisa de vários minutos para estabilizar), e cargas de potência exigiriam proteção elétrica adequada. A senha da Automação~1 está fixa no código, as automações não se comunicam entre si e os textos do display não usam acentos, por limitação do LCD padrão.

\textbf{Melhorias futuras.} Migrar as automações para o ESP32, com Wi-Fi, e integrá-las por MQTT a um painel central; compartilhar a ocupação entre sala, ar-condicionado e iluminação; guardar senhas na EEPROM e adicionar cartões RFID com registro de acessos; incluir um relógio de tempo real para agendas e para o histórico de alarmes; e enviar notificações ao celular da equipe de manutenção quando houver alarme de gás, de temperatura ou de reservatório.

\bibliographystyle{sbc}
\bibliography{referencias}

\appendix
"""


def tabela_componentes(itens):
    linhas = "\n".join(f"{c} & {q} \\\\" for c, q in itens)
    return "\\begin{tabularx}{\\textwidth}{Xc}\n\\toprule\nComponente & Qtd. \\\\\n\\midrule\n" + linhas + "\n\\bottomrule\n\\end{tabularx}"


def secao(n, a):
    letra = ROMANOS[n - 1]
    lig = "\n".join(f"{p} & {c} \\\\" for p, c in a["ligacoes"])
    testes = "\n".join(f"{i} & {e} & {s} & \\conf \\\\" for i, (e, s) in enumerate(a["testes"], 1))
    return rf"""
\subsection{{Automação {n}: {a['titulo']}}}\label{{sub:a{n}}}

\paragraph{{Problema.}} {a['problema']}

\paragraph{{Componentes.}} A Tabela~\ref{{tab:comp{n}}} lista os componentes e a Tabela~\ref{{tab:lig{n}}}, as ligações ao Arduino Uno. A alimentação (5 V e GND) de todos os módulos vem da própria placa.

\begin{{table}}[!htb]
\centering
\caption{{Componentes da Automação {n}}}\label{{tab:comp{n}}}
\small
{tabela_componentes(a['componentes'])}
\end{{table}}

\begin{{table}}[!htb]
\centering
\caption{{Ligações da Automação {n}}}\label{{tab:lig{n}}}
\small
\begin{{tabularx}}{{\textwidth}}{{lX}}
\toprule
Pino do Arduino & Ligado a \\
\midrule
{lig}
\bottomrule
\end{{tabularx}}
\end{{table}}

\paragraph{{Esquema do circuito.}} A Figura~\ref{{fig:circ{n}}} mostra o circuito, desenhado a partir do arquivo \texttt{{diagram.json}} do projeto com as mesmas peças do Wokwi. O projeto completo da simulação está na pasta \simlink{{{a['pasta']}}} do repositório (Apêndice~\ref{{ap:executar}}).

\captura{{figuras/circuito{n}.png}}{{Esquema do circuito da Automação {n}}}{{fig:circ{n}}}

\paragraph{{Lógica.}} {a['logica']} O pseudocódigo resume a decisão:

\begin{{lstlisting}}[style=pseudo]
{a['pseudo']}
\end{{lstlisting}}

\paragraph{{Testes.}} A Tabela~\ref{{tab:teste{n}}} reúne os casos de teste com as entradas aplicadas e as saídas obtidas.

\begin{{table}}[!htb]
\centering
\caption{{Testes da Automação {n}: entradas e saídas}}\label{{tab:teste{n}}}
\small
\begin{{tabularx}}{{\textwidth}}{{cXXc}}
\toprule
\# & Entrada ou ação & Saída esperada & Resultado \\
\midrule
{testes}
\bottomrule
\end{{tabularx}}
\end{{table}}


"""


def apendice(n, a):
    codigo = open(os.path.join(RAIZ, "wokwi", a["pasta"], "sketch.ino"), encoding="utf-8").read().rstrip()
    return rf"""
\section{{Código da Automação {n}: {a['titulo']}}}\label{{ap:a{n}}}

\noindent Projeto do Wokwi: pasta \simlink{{{a['pasta']}}} do repositório.

\noindent Bibliotecas (arquivo \texttt{{libraries.txt}} do projeto): {', '.join(l.strip() for l in open(os.path.join(RAIZ, 'wokwi', a['pasta'], 'libraries.txt'), encoding='utf-8') if l.strip() and not l.startswith('#')) or 'nenhuma além do núcleo do Arduino'}.

\begin{{lstlisting}}[language=C++]
{codigo}
\end{{lstlisting}}
"""


def memoria():
    import re, subprocess
    linhas = []
    saida = subprocess.run([os.path.join(RAIZ, "testes", "compilar_avr.sh")], capture_output=True, text=True, check=True).stdout
    for n, linha in enumerate(l for l in saida.splitlines() if l.startswith("a")):
        m = re.search(r"flash (\d+) B \(([\d.]+)%\).*RAM (\d+) B \(([\d.]+)%\)", linha)
        linhas.append(f"{n + 1}. {AUTOMACOES[n]['titulo']} & {m.group(1)} B ({m.group(2).replace('.', ',')}\\%) & {m.group(3)} B ({m.group(4).replace('.', ',')}\\%) \\\\")
    return "\n".join(linhas)


def main():
    corpo = PREAMBULO + INTRO.replace("%MEMORIA%", memoria())
    corpo += "".join(secao(n, a) for n, a in enumerate(AUTOMACOES, 1))
    corpo += RESTO
    corpo += r"""
\section{Como executar as simulações}\label{ap:executar}

Os nove projetos estão no repositório público deste trabalho no GitHub:

\begin{center}\url{https://github.com/santanathalles76-tech/Trabalho-sistemas-embarcadosA2}\end{center}

\noindent Eles ficam na pasta \texttt{wokwi/}, uma subpasta por automação (\texttt{a1-controle-acesso} a \texttt{a9-painel-senhas}). Cada subpasta contém o programa (\texttt{sketch.ino}), o circuito (\texttt{diagram.json}), as bibliotecas (\texttt{libraries.txt}), o firmware já compilado (\texttt{sketch.hex} e \texttt{sketch.elf}) e o arquivo de configuração \texttt{wokwi.toml}. O arquivo \texttt{README.md} do repositório repete estas instruções.

\paragraph{No navegador.} Em \url{https://wokwi.com}, crie um projeto \emph{Arduino Uno}, cole o conteúdo de \texttt{sketch.ino}, \texttt{diagram.json} e \texttt{libraries.txt} nas abas de mesmo nome e clique no botão verde de iniciar.

\paragraph{No Visual Studio Code.} Instale a extensão \emph{Wokwi Simulator}, abra a subpasta da automação e execute o comando \emph{Wokwi: Start Simulator}. O firmware pré-compilado é carregado pelo \texttt{wokwi.toml}, sem necessidade de compilar.

Durante a simulação, clique nos sensores para alterar seus valores (presença, temperatura, umidade, luminosidade, gás e distância) e nos botões e no teclado para interagir. O monitor serial registra cada decisão do programa. A senha da Automação~1 é 4729; na Automação~7, aguarde os 3 s de calibração antes de aumentar o gás.
"""
    corpo += "".join(apendice(n, a) for n, a in enumerate(AUTOMACOES, 1))
    corpo += "\n\\end{document}\n"
    os.makedirs(os.path.dirname(SAIDA), exist_ok=True)
    open(SAIDA, "w", encoding="utf-8").write(corpo)
    print("gerado", SAIDA)


if __name__ == "__main__":
    main()
