#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define WIFI_SSID "aula"
#define WIFI_SENHA "1234567898"
#define BOT_TOKEN ""

#define sensorGas 33

String pessoa = "";

const unsigned long BOT_MTBS = 1000;

unsigned long bot_lasttime;
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);



float leitorSensorGas()
{
  int saida = analogRead(sensorGas);
  return saida;
}

void verificadorPresencaGas()
{
  if(pessoa != "")
  {
    if(leitorSensorGas() > 1500)
      bot.sendMessage(pessoa, "Alerta de gás!");
  }
}

void mensagensNovas(int numMensagensNovas)
{
  Serial.print("Mensagens Novas: ");
  Serial.println(numMensagensNovas);

  String resposta;
  for(int i = 0; i < numMensagensNovas; i++)
  {
    telegramMessage &msg = bot.messages[i];
    Serial.println("Recebido "+msg.text);
    if(msg.text == "/gas")
      resposta = "Sensor de gás: "+String(leitorSensorGas());
    else if(msg.text == "/avisar")
    {
      pessoa = msg.chat_id;
      resposta = "Ok, vou avisar para você se o gás estiver vazando!";
    }
    else 
      resposta = "Mensagem não compreendida!";
    bot.sendMessage(msg.chat_id, resposta, "Markdown");
  }
}

void testeConexaoInternet()
{
  Serial.print("Testando conexão: ");
  configTime(0, 0, "pool.ntp.org");
  time_t now = time(nullptr);
  while(now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println("Conectado com a internet!");
}

void inicializarWifi()
{
  Serial.print("Conectando ao Wifi ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Adiciona certificado raíz para api.telegram.org
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connectado. Enderenço de IP: ");
  Serial.println(WiFi.localIP());
}

void bot_setup()
{
  const String commands = F("["
                            "{\"command\":\"gas\",  \"description\":\"Verificar o gás\"},"
                            "{\"command\":\"avisar\",  \"description\":\"Faço aviso para você se o gás estiver muito presente no ambiente!\"}"
                            "]");
  bot.setMyCommands(commands);
}

void verificador()
{
  if(millis() - bot_lasttime > BOT_MTBS)
  {
    int numMensagensNovas = bot.getUpdates(bot.last_message_received + 1);

    while(numMensagensNovas)
    {
      Serial.println("Recebido resposta");
      mensagensNovas(numMensagensNovas);
      numMensagensNovas = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}

void setup()
{

  Serial.begin(115200);
  Serial.println();

  inicializarWifi();

  testeConexaoInternet();

  bot_setup();
}

void loop()
{
  verificador();
  verificadorPresencaGas();
}