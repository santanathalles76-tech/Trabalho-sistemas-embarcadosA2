"""Gera o diagram.json e o libraries.txt de cada automação para o Wokwi."""
import json, os

RAIZ = os.path.join(os.path.dirname(__file__), "..", "wokwi")
AUTOR = "Thalles da Silva Santana"

def projeto(pasta, partes, conexoes, bibliotecas):
    base = [{"type": "wokwi-arduino-uno", "id": "uno", "top": 0, "left": 0, "attrs": {}}]
    diagrama = {
        "version": 1, "author": AUTOR, "editor": "wokwi",
        "parts": base + [dict(type=t, id=i, top=y, left=x, attrs=a or {}) for (t, i, x, y, a) in partes],
        "connections": [[a, b, cor, []] for (a, b, cor) in conexoes],
        "dependencies": {},
    }
    os.makedirs(os.path.join(RAIZ, pasta), exist_ok=True)
    with open(os.path.join(RAIZ, pasta, "diagram.json"), "w", encoding="utf-8") as f:
        json.dump(diagrama, f, indent=2, ensure_ascii=False)
    with open(os.path.join(RAIZ, pasta, "libraries.txt"), "w", encoding="utf-8") as f:
        f.write("# Bibliotecas usadas por esta automação (o Wokwi instala sozinho)\n")
        f.write("".join(b + "\n" for b in bibliotecas))

def lcd(x=0, y=-230):
    partes = [("wokwi-lcd1602", "lcd1", x, y, {"pins": "i2c"})]
    conexoes = [("lcd1:GND", "uno:GND.1", "black"), ("lcd1:VCC", "uno:5V", "red"),
                ("lcd1:SDA", "uno:A4", "blue"), ("lcd1:SCL", "uno:A5", "purple")]
    return partes, conexoes

def led(i, pino, cor, x, y):
    """LED com resistor de 220 Ω em série até o GND."""
    partes = [("wokwi-led", f"led{i}", x, y, {"color": cor}),
              ("wokwi-resistor", f"r{i}", x - 5, y + 75, {"value": "220"})]
    conexoes = [(f"uno:{pino}", f"led{i}:A", "green"), (f"led{i}:C", f"r{i}:1", "gray"),
                (f"r{i}:2", "uno:GND.2", "black")]
    return partes, conexoes

def botao(i, pino, cor, x, y):
    """Botão entre o pino e o GND (pull-up interno ativado no código)."""
    return ([("wokwi-pushbutton", f"btn{i}", x, y, {"color": cor})],
            [(f"btn{i}:1.l", f"uno:{pino}", "green"), (f"btn{i}:2.r", "uno:GND.3", "black")])

def buzzer(pino, x, y):
    return ([("wokwi-buzzer", "bz1", x, y, {"volume": "0.1"})],
            [("bz1:1", "uno:GND.3", "black"), ("bz1:2", f"uno:{pino}", "orange")])

def servo(pino, x, y):
    return ([("wokwi-servo", "servo1", x, y, {})],
            [("servo1:GND", "uno:GND.1", "black"), ("servo1:V+", "uno:5V", "red"), ("servo1:PWM", f"uno:{pino}", "orange")])

def pir(pino, x, y):
    return ([("wokwi-pir-motion-sensor", "pir1", x, y, {})],
            [("pir1:VCC", "uno:5V", "red"), ("pir1:GND", "uno:GND.2", "black"), ("pir1:OUT", f"uno:{pino}", "green")])

def rele_com_carga(pino, cor, x, y):
    """Relé acionado pelo Arduino; a carga é um LED ligado no contato NA (NO)."""
    partes = [("wokwi-relay-module", "relay1", x, y, {}),
              ("wokwi-led", "ledcarga", x + 150, y - 10, {"color": cor, "label": "Carga"}),
              ("wokwi-resistor", "rcarga", x + 145, y + 65, {"value": "220"})]
    conexoes = [("relay1:VCC", "uno:5V", "red"), ("relay1:GND", "uno:GND.2", "black"), ("relay1:IN", f"uno:{pino}", "green"),
                ("relay1:COM", "uno:5V", "red"), ("relay1:NO", "ledcarga:A", "orange"),
                ("ledcarga:C", "rcarga:1", "gray"), ("rcarga:2", "uno:GND.2", "black")]
    return partes, conexoes

def juntar(*blocos):
    p, c = [], []
    for bp, bc in blocos:
        p += bp; c += bc
    return p, c

LCD_LIB = ["LiquidCrystal I2C"]

# A1: controle de acesso
p, c = juntar(lcd(), servo(10, 330, -200),
              ([("wokwi-membrane-keypad", "keypad1", 340, -20, {})],
               [("keypad1:R1", "uno:9", "brown"), ("keypad1:R2", "uno:8", "brown"), ("keypad1:R3", "uno:7", "brown"), ("keypad1:R4", "uno:6", "brown"),
                ("keypad1:C1", "uno:5", "gold"), ("keypad1:C2", "uno:4", "gold"), ("keypad1:C3", "uno:3", "gold"), ("keypad1:C4", "uno:2", "gold")]),
              led(1, 12, "green", -150, -60), led(2, 13, "red", -100, -60), buzzer(11, -200, 90))
projeto("a1-controle-acesso", p, c, LCD_LIB + ["Keypad", "Servo"])

# A2: sala de reunião
p, c = juntar(lcd(), pir(2, 330, -60), botao(1, 3, "blue", 330, 120),
              led(1, 9, "green", -170, -60), led(2, 10, "yellow", -120, -60), led(3, 11, "red", -70, -60))
projeto("a2-sala-reuniao", p, c, LCD_LIB)

# A3: ar-condicionado
p, c = juntar(lcd(), pir(2, 330, -60),
              ([("wokwi-ntc-temperature-sensor", "ntc1", -250, 230, {"temperature": "26"})],
               [("ntc1:VCC", "uno:5V", "red"), ("ntc1:GND", "uno:GND.2", "black"), ("ntc1:OUT", "uno:A0", "green")]),
              ([("wokwi-potentiometer", "pot1", 0, 260, {"value": "409"})],
               [("pot1:VCC", "uno:5V", "red"), ("pot1:GND", "uno:GND.2", "black"), ("pot1:SIG", "uno:A1", "green")]),
              rele_com_carga(7, "blue", 330, 120))
projeto("a3-ar-condicionado", p, c, LCD_LIB)

# A4: sala de servidores
p, c = juntar(lcd(), botao(1, 3, "red", 330, 220),
              ([("wokwi-dht22", "dht1", 340, -110, {"temperature": "24", "humidity": "45"})],
               [("dht1:VCC", "uno:5V", "red"), ("dht1:GND", "uno:GND.1", "black"), ("dht1:SDA", "uno:2", "green")]),
              rele_com_carga(7, "white", 330, 90), buzzer(8, -230, 90),
              led(1, 9, "green", -170, -60), led(2, 10, "yellow", -120, -60), led(3, 11, "red", -70, -60))
projeto("a4-sala-servidores", p, c, LCD_LIB + ["DHT sensor library", "Adafruit Unified Sensor"])

# A5: contador de pessoas e lotação
p, c = juntar(lcd(), botao(1, 2, "green", 330, 0), botao(2, 3, "red", 330, 90), servo(6, 330, -200), buzzer(8, -230, 90),
              led(1, 9, "green", -170, -60), led(2, 10, "yellow", -120, -60), led(3, 11, "red", -70, -60))
projeto("a5-controle-lotacao", p, c, LCD_LIB + ["Servo"])

# A6: iluminação com luz natural
p, c = juntar(lcd(),
              ([("wokwi-photoresistor-sensor", "ldr1", 330, 40, {})],
               [("ldr1:VCC", "uno:5V", "red"), ("ldr1:GND", "uno:GND.2", "black"), ("ldr1:AO", "uno:A0", "green")]),
              led(1, 9, "white", -120, -60))
projeto("a6-iluminacao-natural", p, c, LCD_LIB)

# A7: vazamento de gás
p, c = juntar(([("wokwi-gas-sensor", "gas1", 330, -80, {})],
               [("gas1:VCC", "uno:5V", "red"), ("gas1:GND", "uno:GND.1", "black"), ("gas1:AOUT", "uno:A0", "green"), ("gas1:DOUT", "uno:4", "blue")]),
              botao(1, 3, "black", 330, 60), servo(6, -10, -230), rele_com_carga(7, "cyan", 330, 150),
              buzzer(8, -230, 90), led(1, 9, "red", -120, -60))
projeto("a7-vazamento-gas", p, c, ["Servo"])

# A8: reservatório de água
p, c = juntar(lcd(),
              ([("wokwi-hc-sr04", "sonar1", 330, -120, {"distance": "150"})],
               [("sonar1:VCC", "uno:5V", "red"), ("sonar1:GND", "uno:GND.1", "black"), ("sonar1:TRIG", "uno:12", "orange"), ("sonar1:ECHO", "uno:13", "blue")]),
              rele_com_carga(7, "blue", 330, 60), buzzer(8, -260, 90),
              led(1, 2, "red", -230, -60), led(2, 3, "yellow", -180, -60), led(3, 4, "green", -130, -60), led(4, 5, "blue", -80, -60))
projeto("a8-reservatorio-agua", p, c, LCD_LIB)

# A9: painel de senhas
p, c = juntar(lcd(), botao(1, 2, "blue", 330, -40), botao(2, 3, "yellow", 330, 50), botao(3, 4, "green", 330, 140),
              buzzer(8, -230, 90), led(1, 9, "orange", -120, -60))
projeto("a9-painel-senhas", p, c, LCD_LIB)
print("ok")
