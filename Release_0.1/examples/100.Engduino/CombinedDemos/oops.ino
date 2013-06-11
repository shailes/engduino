void oops(int success) {
  colour c[16] = {OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF};

  // Punishment
  //
  for (int i = 0; i < 2; i++) {
    EngduinoLEDs.setAll(RED);
    delay(500);
    EngduinoLEDs.setAll(OFF);
    delay(500);    
  }

  // Reward
  //
  for (int i = 0; i < success; i++) {
    c[i] = colours[i];
    EngduinoLEDs.setLEDs(c);
    delay(500);
  }
  EngduinoLEDs.setAll(OFF);
  delay(500);    
  EngduinoLEDs.setLEDs(c);
  delay(500);
  EngduinoLEDs.setAll(OFF);
  delay(500);    
  EngduinoLEDs.setLEDs(c);
  delay(500);
  EngduinoLEDs.setAll(OFF);
  
  EngduinoButton.reset();  // Clear the wasPressed state   
}
