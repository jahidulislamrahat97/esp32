
#include <WiFi.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "EduBlock.co"
#define WIFI_PASSWORD "12345STEMClub"
#define FIREBASE_HOST "led-control-ce1b2.firebaseio.com" //Do not include https:// in FIREBASE_HOST
#define FIREBASE_AUTH "NaODGSOCMYngnEJp258i8Crp2Lq0w07Kcg1QX69L"


//Define Firebase Data object
FirebaseData firebaseData;

void printResult(FirebaseData &data);

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  String path = "/Test/Json";

  String jsonStr = "";

  FirebaseJson json1;

  FirebaseJsonData jsonObj;
  int n;
  for ( int i = 1; i <= 1000; i++ ) {
    n = i ;
  }


  double x = 0.92813 ;
  json1.set("Hi/myInt", n);
  json1.set("Hi/myDouble", x);


  Serial.println("------------------------------------");
  Serial.println("JSON Data");
  json1.toString(jsonStr, true);
  Serial.println(jsonStr);
  Serial.println("------------------------------------");


  //*********** send data


  Serial.println("------------------------------------");
  Serial.println("Set JSON test...");

  if (Firebase.set(firebaseData, path, json1))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.print("VALUE: ");
    printResult(firebaseData);
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }




  // ************get data




  Serial.println("------------------------------------");
  Serial.println("Get JSON test...");
  if (Firebase.get(firebaseData, path))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.print("VALUE: ");
    if (firebaseData.dataType() == "json")
    {
      jsonStr = firebaseData.jsonString();
      printResult(firebaseData);
    }

    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Serial.println("------------------------------------");
  Serial.println("Try to parse return data and get value..");

  json1.setJsonData(jsonStr);
  json1.get(jsonObj, "Hi/myDouble");
  Serial.print("Hi/myDouble: ");
  Serial.println(jsonObj.doubleValue, 4);

  Serial.println("------------------------------------");
  Serial.println();
}

void printResult(FirebaseData &data)
{

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson &json = data.jsonObject();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == JSON_OBJECT ? "object" : "array");
      if (type == JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json.iteratorEnd();
  }

}

void loop()
{
}
