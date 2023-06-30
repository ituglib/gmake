/* Test file for tandem_ext.c */
#include <stdio.h>
#include <ctype.h>
#include "../tandem_ext.c"

void testToGuardian(const char *node, const char *volume) {
	char ossName[64];
	char guardianName[64];

	printf("ToGuardian:\n");

	/* Protective tests */
	printf("%s\n", toGuardian(guardianName, NULL));
	printf("%s\n", toGuardian(guardianName, "."));
	printf("%s\n", toGuardian(guardianName, "./"));

	/* Guardian local paths. */
	printf("%s\n", toGuardian(guardianName, "/G"));
	printf("%s\n", toGuardian(guardianName, "/G/"));
	snprintf(ossName, sizeof(ossName), "/G/%s", volume);
	printf("%s\n", toGuardian(guardianName, ossName));
	snprintf(ossName, sizeof(ossName), "/G/%s/subvol", volume);
	printf("%s\n", toGuardian(guardianName, ossName));
	snprintf(ossName, sizeof(ossName), "/G/%s/subvol/file", volume);
	printf("%s\n", toGuardian(guardianName, ossName));

	/* Expand paths. */
	snprintf(ossName, sizeof(ossName), "/E/%s", node);
	printf("%s\n", toGuardian(guardianName, ossName));
	snprintf(ossName, sizeof(ossName), "/E/%s/G", node);
	printf("%s\n", toGuardian(guardianName, ossName));
	snprintf(ossName, sizeof(ossName), "/E/%s/G/%s", node, volume);
	printf("%s\n", toGuardian(guardianName, ossName));
	snprintf(ossName, sizeof(ossName), "/E/%s/G/%s/subvol", node, volume);
	printf("%s\n", toGuardian(guardianName, ossName));
	snprintf(ossName, sizeof(ossName), "/E/%s/G/%s/subvol/file", node, volume);
	printf("%s\n", toGuardian(guardianName, ossName));
}

void testToOss(const char *node, const char *volume) {
	char ossName[64];
	char guardianName[64];

	printf("ToOSS:\n");

	/* Protective tests */
	printf("%s\n", toOss(ossName, sizeof(ossName), NULL));
	snprintf(guardianName, sizeof(guardianName), "\\");
	printf("%s\n", toOss(ossName, sizeof(ossName), guardianName));

	/* Guardian local paths. */
	snprintf(guardianName, sizeof(guardianName), "$%s", volume);
	printf("%s\n", toOss(ossName, sizeof(ossName), guardianName));
	snprintf(guardianName, sizeof(guardianName), "$%s.Subvol", volume);
	printf("%s\n", toOss(ossName, sizeof(ossName), guardianName));
	snprintf(guardianName, sizeof(guardianName), "$%s.Subvol.File", volume);
	printf("%s\n", toOss(ossName, sizeof(ossName), guardianName));

	/* Expand paths. */
	snprintf(guardianName, sizeof(guardianName), "\\%s", node);
	printf("%s\n", toOss(ossName, sizeof(ossName), guardianName));
	snprintf(guardianName, sizeof(guardianName), "\\%s.", node);
	printf("%s\n", toOss(ossName, sizeof(ossName), guardianName));
	snprintf(guardianName, sizeof(guardianName), "\\%s.$%s", node, volume);
	printf("%s\n", toOss(ossName, sizeof(ossName), guardianName));
	snprintf(guardianName, sizeof(guardianName), "\\%s.$%s.Subvol", node, volume);
	printf("%s\n", toOss(ossName, sizeof(ossName), guardianName));
	snprintf(guardianName, sizeof(guardianName), "\\%s.$%s.Subvol.File", node, volume);
	printf("%s\n", toOss(ossName, sizeof(ossName), guardianName));
}

/**
 * Unit test driver.
 * @param argc the number of arguments. Must be 3.
 * @param argv the argument list. argv[1] is the node. argv[2] is the volume.
               Both are stripped of special characters.
 * @return the completion code. 0 if normal exit.
 */
int main(int argc, char **argv) {
	testToGuardian(argv[1], argv[2]);
	testToOss(argv[1], argv[2]);

	return 0;
}
