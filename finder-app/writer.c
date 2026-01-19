#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	
	// Open syslog
    openlog(NULL, 0, LOG_USER);
    if (argc != 3) {
		fprintf(stderr, "Usage: %s <filename> <string>\n", argv[0]);
        syslog(LOG_ERR, "Usage: %s <filename> <string>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *filename = argv[1];
    const char *text = argv[2];    

    // Log what we are going to do
    syslog(LOG_DEBUG, "Writing \"%s\" to %s", text, filename);

    // Open the file for writing
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        syslog(LOG_ERR, "Error opening file %s: %s", filename, strerror(errno));
        closelog();
        return EXIT_FAILURE;
    }

    // Write string to file
    if (fputs(text, file) == EOF) {
        syslog(LOG_ERR, "Error writing to file %s: %s", filename, strerror(errno));
        fclose(file);
        closelog();
        return EXIT_FAILURE;
    }

    // Close file
    if (fclose(file) != 0) {
        syslog(LOG_ERR, "Error closing file %s: %s", filename, strerror(errno));
        closelog();
        return EXIT_FAILURE;
    }

    // Success message to stdout (optional)
    printf("Text written to %s successfully.\n", filename);

    // Close syslog
    closelog();
    return EXIT_SUCCESS;
}
