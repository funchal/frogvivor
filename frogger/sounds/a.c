#include <stdlib.h>
#include <AL/alut.h>

int
main (int argc, char **argv)
{
  ALuint helloBuffer, helloSource;
  alutInit (&argc, argv);
  helloBuffer = alutCreateBufferFromFile ("4.wav");
  alGenSources (1, &helloSource);
  alSourcei (helloSource, AL_BUFFER, helloBuffer);
  alSourcePlay (helloSource);
  alutSleep (4); //you need to put here the length in seconds of the file
  alutExit ();
  return EXIT_SUCCESS;
}
