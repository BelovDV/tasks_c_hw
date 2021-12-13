/*

special characters
'?' next is unnecessary, set next result to ptr to it if finded
'!' next is necessary, set next result to ptr to it if finded
' ' may be one of delimiters (passed separately)
'\t' should be one of delimiters (passed separately)
'$' next is common symbol
// '#' next is number of rule (1-9) (rule 0 is main)
// '@' boards of inner definition of rule
'*' greed repeating next
'[]' set of elements

plans
? - may end checking on this point (if str is '\0' further)

example
is next string [rx+k*ry+c] (r1+4*rbp);(4*rip+123);([1]);([rbp]);...
rule 0: "?$[?#1?+?(1|2|4|8)?$*?#1?+?[1-9]*[0-9]?$]"
rule 1: "r?([0-9]|?(10)|?(11)|((?i?b?s)p|f)"

in my point of view, '?' is insight as i never heard of it before
*/

#ifndef HEADER_REGEX
#define HEADER_REGEX

typedef struct
{
    char *rule;             // pointer to main rule
    char **additional_rule; // pointer to set of rules
    char **result;          // pointer to dest of '?', should be allocated
    char *considered;       // string which one to process

    int flags; // inner
} Regex;

/**
 * @brief parse string 'iter' by rules 'rule' and save result
 * @return is_error
 */
int regex(Regex *regex);

#endif
