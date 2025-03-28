#include <Arduino.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define GPIO0 0
#define GPIO2 2
#define TX 1
#define RX 3

#define SSIDAP "ESP01-01"
#define PASSAP "321321321"
#define SSID "Piso1NSNet"
#define PASS "Panconqueso2.4"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

boolean btnA = false;
boolean btnB = false;
boolean right = false;
boolean left = false;

String ip2Str(IPAddress ip)
{
  String s = "";
  for (int i = 0; i < 4; i++)
  {
    s += i ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="mul">
	<head>
		<meta
			name="viewport"
			content="width=device-width, initial-scale=1.0, user-scalable=no"
		/>
		<title>joystick</title>
		<style>
			body {
				display: flex;
				justify-content: center;
				align-items: center;
				flex-direction: column;
				background-color: #333;
				min-height: 75vh;
			}
			.j__container {
				display: flex;
				justify-content: space-between;
				align-items: center;
				width: 35em;
				height: 7em;
				-webkit-touch-callout: none;
				-webkit-user-select: none;
				-khtml-user-select: none;
				-moz-user-select: none;
				-ms-user-select: none;
				user-select: none;
			}
			.j__lscontainer {
				width: 6em;
				height: 4em;
				border-radius: 1em;
				background-color: #222;
				display: flex;
				justify-content: center;
				align-items: center;
			}
			.j__stick {
				width: 4em;
				height: 4em;
				background-color: firebrick;
				border-radius: 1em;
			}
			.j__btns {
				width: 7em;
				height: 7em;
				position: relative;
			}
			.j__btnA {
				background-color: green;
				bottom: 0;
				left: 0;
			}
			.j__btnB {
				background-color: red;
				top: 0;
				right: 0;
			}
			.j__btnA,
			.j__btnB {
				position: absolute;
				display: flex;
				justify-content: center;
				align-items: center;
				font-family: sans-serif;
				font-weight: bold;
				color: white;
				border-radius: 56px;
				width: 56px;
				height: 56px;
				box-shadow: 0 0 0.5em black;
				font-size: 1.5em;
			}
			.info {
				color: white;
				font-size: 2em;
				font-weight: bold;
			}
			.infoIps {
				color: white;
				font-size: 1em;
				font-weight: bold;
				margin-bottom: 1.5em;
				font-style: italic;
			}
			.infoIps span {
				color: green;
			}
		</style>
	</head>
	<body>
		<div class="infoIps">
			<div>
				IP_AP:
				<span>192.168.4.1</span>
			</div>
			<div>
				IP_STA:
				<span id="ipSta">0.0.0.0</span>
			</div>
		</div>
		<div class="j__container" draggable="false">
			<div class="j__lscontainer">
				<div id="lstick" class="j__stick"></div>
			</div>
			<div>
				<div id="stickposition" class="info">
					0,0
				</div>
				<div id="infobtna" class="info">
					-
				</div>
				<div id="infobtnb" class="info">
					-
				</div>
			</div>

			<div class="j__btns">
				<div id="btnA" class="j__btnA">A</div>
				<div id="btnB" class="j__btnB">B</div>
			</div>
		</div>
		<script>
			let ipSta = '0.0.0.0';
			let infoIpSta = document.getElementById('ipSta');
			let gateway = `ws://${window.location.hostname}/ws`;
			let websocket = new WebSocket(gateway);
			websocket.onmessage = function (event) {
				console.log(event.data);
				if (event.data.includes('STA_IP')) {
					ipSta = event.data.split(';')[1].split('STA_IP:')[1];
					infoIpSta.innerHTML = ipSta;
					if (ipSta === '0.0.0.0') {
						infoIpSta.style.color = 'red';
					} else {
						infoIpSta.style.color = 'green';
					}
				}
			};
			function sendMsg(msg) {
				if (!websocket.readyState) {
					return;
				}
				websocket.send(msg);
			}
			class stick {
				constructor(id, maxX = 0, maxY = 0) {
					this.value = { x: 0, y: 0 };
					this.element = document.getElementById(id);
					this.active = false;
					this.dragStart = null;
					this.touchId = null;
					const click = (e) => {
						this.active = true;
						e.preventDefault();
						if (e.changedTouches) {
							this.dragStart = {
								x: e.changedTouches[0].clientX,
								y: e.changedTouches[0].clientY,
							};
							this.touchId = e.changedTouches[0].identifier;
						} else {
							this.dragStart = {
								x: e.clientX,
								y: e.clientY,
							};
						}
					};
					const move = (e) => {
						if (!this.active) return;
						let touchmoveId = null;
						if (e.changedTouches) {
							for (let i = 0; i < e.changedTouches.length; i++) {
								if (e.changedTouches[i].identifier === this.touchId) {
									touchmoveId = i;
									e.clientX = e.changedTouches[i].clientX;
									e.clientY = e.changedTouches[i].clientY;
								}
							}
							if (touchmoveId === null) return;
						}

						let xDiff = Math.floor(e.clientX - this.dragStart.x);
						let yDiff = Math.floor(e.clientY - this.dragStart.y);
						Math.abs(xDiff) > maxX && (xDiff = maxX * Math.sign(xDiff));
						Math.abs(yDiff) > maxY && (yDiff = maxY * Math.sign(yDiff));
						this.value = {
							x: xDiff,
							y: yDiff,
						};
						this.element.style.transform = `translate(${xDiff}px, ${yDiff}px)`;
					};
					const up = (e) => {
						this.value = {
							x: 0,
							y: 0,
						};
						if (!this.active) return;
						if (
							e.changedTouches &&
							this.touchId !== e.changedTouches[0].identifier
						)
							return;
						this.element.style.transform = 'translate(0, 0)';
						this.touchId = null;
						this.active = false;
					};

					document.addEventListener('mouseup', up);
					this.element.addEventListener('touchend', up);
					this.element.addEventListener('mousedown', click);
					this.element.addEventListener('touchstart', click);
					document.addEventListener('mousemove', move);
					this.element.addEventListener('touchmove', move);
				}
			}
			class button {
				constructor(id) {
					this.element = document.getElementById(id);
					this.active = false;
					const click = (e) => {
						this.active = true;
						e.preventDefault();
					};
					const up = (e) => {
						this.active = false;
					};
					document.addEventListener('mouseup', up);
					this.element.addEventListener('touchend', up);
					this.element.addEventListener('mousedown', click);
					this.element.addEventListener('touchstart', click);
				}
			}
			const infoStick = document.getElementById('stickposition');
			const infoBtnA = document.getElementById('infobtna');
			const infoBtnB = document.getElementById('infobtnb');
			let stickA = new stick('lstick', 64);
			let btnA = new button('btnA');
			let btnB = new button('btnB');
			let lastBtnA = false;
			let lastBtnB = false;
			let lastStick = { x: null, y: null };
			const loop = () => {
				//button and stick behavior control
				if (lastBtnA !== btnA.active) {
					lastBtnA = btnA.active;
					if (btnA.active) {
						sendMsg('A+');
						btnA.element.style = 'transform: scale(0.9);';
						//vibrate for movile devices if button is pressed
						window.navigator.vibrate(60);
					} else {
						sendMsg('A-');
						btnA.element.style = 'transform: scale(1);';
					}
				}
				if (lastBtnB !== btnB.active) {
					lastBtnB = btnB.active;
					if (btnB.active) {
						sendMsg('B+');
						btnB.element.style = 'transform: scale(0.9);';
						window.navigator.vibrate(60);
					} else {
						sendMsg('B-');
						btnB.element.style = 'transform: scale(1);';
					}
				}
				if (lastStick.x !== stickA.value.x || lastStick.y !== stickA.value.y) {
					sendMsg('X:' + stickA.value.x);
					lastStick = { x: stickA.value.x, y: stickA.value.y };
				}
				requestAnimationFrame(loop);
				infoStick.innerHTML = `${Math.round(stickA.value.x)}, ${Math.round(
					stickA.value.y
				)}`;
				infoBtnA.innerHTML = btnA.active ? 'A' : '-';
				infoBtnB.innerHTML = btnB.active ? 'B' : '-';
			};
			loop();
		</script>
	</body>
</html>
)rawliteral";

void notifyClients(String msg)
{
  ws.textAll(msg);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (data[0] == 'A')
    {
      if (strcmp((char *)data, "A-") == 0)
      {
        btnA = false;
      }
      else
      {
        btnA = true;
      }
      // Serial.println(btnA);
      // Serial.println(AP_IP);
      // Serial.println(STA_IP);
      digitalWrite(RX, btnA);
    }
    if (data[0] == 'B')
    {
      if (strcmp((char *)data, "B-") == 0)
      {
        btnB = false;
      }
      else
      {
        btnB = true;
      }
      // Serial.println(btnB);
      digitalWrite(TX, btnB);
    }
    // data uint8_t include X:
    if (data[0] == 'X')
    {
      // substr after X:
      int x = 0;
      x = atoi((char *)&data[2]);
      if (x == 0)
      {
        right = false;
        left = false;
      }
      else if (x > 0)
      {
        right = true;
        left = false;
      }
      else
      {
        right = false;
        left = true;
      }
      digitalWrite(GPIO0, left);
      digitalWrite(GPIO2, right);
    }
  }
}
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    notifyClients("AP_IP:" + ip2Str(WiFi.softAPIP()) + ";STA_IP:" + ip2Str(WiFi.localIP()));
    break;
  case WS_EVT_DISCONNECT:
    // Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String &var)
{
  // Serial.println(var);
  return String();
}

void setup()
{
  pinMode(TX, OUTPUT);
  pinMode(RX, OUTPUT);
  pinMode(GPIO0, OUTPUT);
  pinMode(GPIO2, OUTPUT);

  digitalWrite(TX, false);
  digitalWrite(RX, false);
  digitalWrite(GPIO0, false);
  digitalWrite(GPIO2, false);

  // Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);

  // AP
  WiFi.softAP(SSIDAP, PASSAP);
  while (!WiFi.softAP(SSIDAP, PASSAP))
  {
    delay(100);
  }
  // STA
  bool conectado = true;
  WiFi.begin(SSID, PASS);
  byte intentos = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    intentos++;
    delay(1000);
    if (intentos > 10)
    {
      // Serial.println("No se pudo conectar");
      conectado = false;
      break;
    }
  }

  // Serial.println("MODO AP");
  // Serial.println("Access point Creado");
  // Serial.print("IP address host: ");
  // Serial.println(WiFi.softAPIP());
  if (conectado)
  {
    // Serial.println("MODO STA");
    // Serial.println("Conectado a " + String(SSID));
    // Serial.println("IP address: ");
    // Serial.println(WiFi.localIP());
  }
  else
  {
    // Serial.println("MODO STA");
    // Serial.println("No se pudo conectar a: " + String(SSID));
  }
  initWebSocket();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html, processor); });
  server.begin();
}

void loop()
{
  ws.cleanupClients();
}
