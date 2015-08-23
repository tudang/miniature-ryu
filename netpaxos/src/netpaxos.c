#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "netpaxoslearner.h"
#include "netpaxosclient.h"
#include "config.h"

void usage(char* prog) {
    fprintf(stderr, "Usage: %s -c/-s -t interval -n num_instances eth*\n", prog);
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
    while ((c = getopt (argc, argv, "cst:n:")) != -1)
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
        usage(argv[0]);
        return 1;
      default:
        abort ();
        usage(argv[0]);
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
        run_learner(cols, num_packets, interfaces);
    }
  return 0;
}
