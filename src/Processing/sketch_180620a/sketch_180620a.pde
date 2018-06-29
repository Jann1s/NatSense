import controlP5.*;
import processing.serial.*;

Serial myPort;

ControlP5 cp5;

String arrStored[] = {"No Data", "No Data", "No Data"};
String temp[] = {"", "", ""};

void setup(){
  
  size(800,450);
  
  printArray(Serial.list());
  myPort = new Serial(this, "/dev/cu.usbserial-A50285BI", 115200);
  myPort.bufferUntil('\n');
}

void serialEvent(Serial p) { 
  String inBuffer = p.readString();   
  
  if (inBuffer != null && inBuffer != "") {
      arrStored = split(inBuffer, ';');
  }
} 

void draw(){
  
  if (temp[0] != arrStored[0]) {
     temp = arrStored;
     println(arrStored[0] + ';' + arrStored[1] + ';' + arrStored[2]); 
  }
   
  background(33,150,243);
  textSize(50);
  fill(0,0,0);
  
  text("Speed of Obj.:", 20, 100);
  text("Pedestrians:", 20, 150);
  text("Cyclists:", 20, 200);
  
  text(arrStored[0] + " km/h", 400, 100);
  text(arrStored[1], 400, 150);
  text(arrStored[2], 400, 200);
  
  /*
  //text("Radar Data",20,50);
  if (port.read() == -1){
    //text("No input",20,100);
   }else{
     String sVars = port.readString();
     String arr[] = {"","",""};
     if (sVars != null && sVars != "") {
       arr = split(sVars, ';');
       arrStored[0] = arr[0];
       arrStored[1] = arr[1];
       arrStored[2] = arr[2];
     }
     
     //text(arr[0], 20, 100); //<>//
     //text(arr[1], 20, 150);
     //text(arr[2], 20, 200);
    //8char*[] vars = char(port.read()); 
    //text(vars,20,100);
    
    delay(100);
  }
  */
}  
