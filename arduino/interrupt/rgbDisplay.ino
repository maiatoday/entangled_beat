void setPixelColor(byte r, byte g, byte b)
{
  analogWrite(ledR, r);
  analogWrite(ledG, g);
  analogWrite(ledB, b);
}


//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void setPixelColor(uint32_t c)
{
  byte r,g,b;
   
  b=(byte)c;
  c>>=8;
  g=(byte)c;
  c>>=8;
  r=(byte)c;
  
  setPixelColor(r,g,b);
}



// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}
