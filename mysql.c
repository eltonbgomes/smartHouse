#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress server_addr(54,39,75,7); // O IP DO SERVIDOR DA CLEVER CLOUD
char user[] = "usrxplt5d3lv7sfi";  // Usuario MySQL
char password[] = "BhkiXAfdcVRh3q55Paou"; //   Senha MySQL

//IPAddress server_addr(10,0,1,35);  // IP of the MySQL *server* here
//char user[] = "root";              // MySQL user login username
//char password[] = "secret";        // MySQL user login password

// Sample query
char INSERT_SQL[] = "INSERT INTO bdqyngbnbsudmj189t37.temperatura (temperatura) VALUES (435.1)";

EthernetClient client;
MySQL_Connection conn((Client *)&client);

void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for serial port to connect
  Ethernet.begin(mac_addr);
  Serial.println("Connecting...");
  
  if (conn.connect(server_addr, 3306, user, password)) {
    delay(1000);
  }
  else
    Serial.println("Connection failed.");
}


void loop() {

  Serial.println("Recording data.");

  // Initiate the query class instance
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  // Execute the query
  Serial.println(INSERT_SQL);
  cur_mem->execute(INSERT_SQL);
  // Note: since there are no results, we do not need to read any data
  // Deleting the cursor also frees up memory used
  delete cur_mem;
  delay(5000);
}