#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_MATCHES 1 //The maximum number of matches allowed in a single string

char* mysubstr(char *buff, int start,int len, char* substr)
{
    strncpy(substr, buff+start, len);
    substr[len] = 0;
    return substr;
}
 
int mymatch(regex_t *pexp, char *sz) {
	regmatch_t matches[MAX_MATCHES];
	if (regexec(pexp, sz, MAX_MATCHES, matches, 0) == 0) {
		char *result;
		result = mysubstr(sz, 3, (matches[0].rm_eo-matches[0].rm_so-3), result);
		return atoi(result);
	} 
	return -1; // does not match
}
/*
int main() {
	int rv;
	regex_t exp;
	rv = regcomp(&exp, "-?obj[0-9]+?", REG_EXTENDED);
	if (rv != 0) {
		printf("regcomp failed with %d\n", rv);
	}
	//2. Now run some tests on it
	printf("%d\n", mymatch(&exp, "obj2_90"));

	//3. Free it
	regfree(&exp);
	return 0;
}*/
