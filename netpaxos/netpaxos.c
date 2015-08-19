#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void usage(char* prog) {
    fprintf(stderr, "Usage: %s eth*\n", prog);
     exit(1);
}

int main(int argc, char**argv)
{
    int cflag = 0;
    int sflag = 0;
    int interval = 1000;
    int num_packets = 1;
    int index;
    int c;
    while ((c = getopt (argc, argv, "cs:t:n:")) != -1)
    switch (c)
      {
      case 'c':
        cflag = 1;
        break;
      case 's':
        sflag = 1;
        break;
      case 't':
        interval = atoi(optarg);
        break;
      case 'n':
        num_packets = atoi(optarg);
        break;
      case '?':
        if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }

    if (cflag) {
        run_client(interval, num_packets);
    }
    else if (sflag) {
        int cols = argc - optind;
        char *interfaces[cols];
        for (index = optind; index < argc; index++) {
            interfaces[index - optind] = strdup(argv[index]);
        }
        run_learner(cols, interfaces);
    }
  return 0;
}
