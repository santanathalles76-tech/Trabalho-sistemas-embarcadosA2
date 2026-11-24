# Escritório Inteligente: nove automações com Arduino Uno

Trabalho opcional da A2 da disciplina de **Sistemas Embarcados** (Prof. Caio Silva Azeredo), Ciência da Computação, Universidade Veiga de Almeida.

**Autor:** Thalles da Silva Santana (1240110579)

Automação simulada de um escritório corporativo com nove sistemas embarcados. Todos usam um Arduino Uno (ATmega328P), são escritos em C++ e foram simulados no [Wokwi](https://wokwi.com).

## Automações

| # | Automação | Entradas | Saídas | Pasta |
|---|---|---|---|---|
| 1 | Controle de acesso por senha | Teclado 4x4 | Servo (trava), LEDs, buzzer, LCD | [`wokwi/a1-controle-acesso`](wokwi/a1-controle-acesso) |
| 2 | Sala de reunião livre, reservada ou ocupada | PIR, botão | LEDs, LCD | [`wokwi/a2-sala-reuniao`](wokwi/a2-sala-reuniao) |
| 3 | Ar-condicionado por ocupação e temperatura | NTC, potenciômetro, PIR | Relé, LCD | [`wokwi/a3-ar-condicionado`](wokwi/a3-ar-condicionado) |
| 4 | Monitoramento da sala de servidores | DHT22, botão | LEDs, buzzer, relé, LCD | [`wokwi/a4-sala-servidores`](wokwi/a4-sala-servidores) |
| 5 | Contador de pessoas e lotação do auditório | 2 botões | Servo (catraca), LEDs, buzzer, LCD | [`wokwi/a5-controle-lotacao`](wokwi/a5-controle-lotacao) |
| 6 | Iluminação com aproveitamento da luz natural | LDR | LED com PWM, LCD | [`wokwi/a6-iluminacao-natural`](wokwi/a6-iluminacao-natural) |
| 7 | Detecção de vazamento de gás na copa | MQ-2, botão | Servo (válvula), relé, LED, buzzer | [`wokwi/a7-vazamento-gas`](wokwi/a7-vazamento-gas) |
| 8 | Nível do reservatório e bomba de recalque | HC-SR04 | Relé, 4 LEDs, buzzer, LCD | [`wokwi/a8-reservatorio-agua`](wokwi/a8-reservatorio-agua) |
| 9 | Painel de senhas com atendimento preferencial | 3 botões | LCD, buzzer, LED | [`wokwi/a9-painel-senhas`](wokwi/a9-painel-senhas) |

## Como executar

Cada pasta em `wokwi/` é um projeto completo do Wokwi:

| Arquivo | Conteúdo |
|---|---|
| `sketch.ino` | programa comentado |
| `diagram.json` | circuito |
| `libraries.txt` | bibliotecas |
| `sketch.hex`, `sketch.elf` | firmware já compilado para o ATmega328P |
| `wokwi.toml` | configuração da extensão do VS Code |

### Opção 1: no navegador

1. Em [wokwi.com](https://wokwi.com), crie um projeto **Arduino Uno**.
2. Cole o conteúdo de `sketch.ino`, `diagram.json` e `libraries.txt` nas abas de mesmo nome.
   - O `libraries.txt` é criado pela seta ao lado das abas.
3. Clique no botão verde de play.

### Opção 2: no VS Code

1. Instale a extensão **Wokwi Simulator**.
2. Abra a pasta da automação (por exemplo, `wokwi/a1-controle-acesso`).
3. Pressione `F1` e escolha **Wokwi: Start Simulator**.

O firmware pré-compilado é carregado pelo `wokwi.toml`.

### Durante a simulação

- Clique no PIR para simular movimento.
- Clique no NTC, no DHT22, no LDR, no MQ-2 ou no HC-SR04 para ajustar temperatura, umidade, luz, gás e distância.
- Use os botões e o teclado.
- O monitor serial registra cada decisão do programa.

A senha da automação 1 é **4729**. Na automação 7, espere 3 s depois de iniciar (calibração com ar limpo) antes de aumentar o gás.

## Estrutura

```
wokwi/        um projeto do Wokwi por automação (a1 a a9), com firmware compilado
artigo/       artigo no template SBC (main.tex, referencias.bib)
testes/       testes automatizados da lógica e compilação para o ATmega328P
ferramentas/  scripts que geram os diagram.json e o main.tex
```

## Testes

```bash
# Testes de lógica (84 verificações, só precisa de g++)
./testes/rodar_testes.sh

# Compilação para o ATmega328P com o núcleo oficial do Arduino
# (precisa de avr-gcc e avr-libc; baixa o núcleo e as bibliotecas na 1a execução)
./testes/compilar_avr.sh
```

Os testes de lógica rodam o próprio `sketch.ino` de cada automação no computador, com uma simulação mínima das funções do Arduino, e reproduzem as tabelas de entrada e saída do artigo.

## Artigo

O artigo completo, com o código no apêndice, está em `artigo/`. Para compilar, use o Overleaf ou `latexmk -pdf main.tex` dentro da pasta.
