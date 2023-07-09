#include <stdio.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
  // Setup syslog logging with LOG_USER facility
  openlog(argv[0], LOG_PID | LOG_CONS, LOG_USER);

  // Check if the required arguments are provided
  if (argc < 3) {
    syslog(LOG_PERROR, "Insufficient arguments provided.");
    syslog(LOG_PERROR, "Usage: %s <writefile> <writestr>\n", argv[0]);
    
    fprintf(stderr, "Error: Insufficient arguments provided.\n");
    fprintf(stderr, "Usage: %s <writefile> <writestr>\n", argv[0]);

    closelog();
    return 1;
  }

  // Store the arguments in variables
  const char *writefile = argv[1];
  const char *writestr = argv[2];

  // Open the file in write mode
  FILE *file = fopen(writefile, "w");
  if (file == NULL) {
    syslog(LOG_PERROR, "Error: Failed to open the file '%s'.\n", writefile);

    fprintf(stderr, "Error: Failed to open the file '%s'.\n", writefile);

    closelog();
    return 1;
  }

  // Write the string to the file
  fprintf(file, "%s", writestr);

  // Close the file
  fclose(file);


  // Log the message: "Writing <string> to <file>"
  syslog(LOG_DEBUG, "Writing %s to %s", writestr, writefile);

  // Close syslog
  closelog();

  return 0;
}
