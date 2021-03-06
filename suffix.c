#include "links.h"

#include "suffix.inc"

#include "suffix_x.inc"

#define array_elements(a)	(sizeof(a) / sizeof(*a))

static int search_list(const_char_ptr const *list, int len, unsigned char *name)
{
	int result;
#define T_EQUAL(n, k)		!casestrcmp(cast_uchar list[n], k)
#define T_ABOVE(n, k)		casestrcmp(cast_uchar list[n], k) > 0
	BIN_SEARCH(len, T_EQUAL, T_ABOVE, name, result);
	return result != -1;
}

static int search_list_and_wildcards(const_char_ptr const *list, int len, unsigned char *name)
{
	unsigned char *dot, *x;
	int sl;

	if (search_list(list, len, name)) return 1;

	x = stracpy(cast_uchar "*.");
	add_to_strn(&x, name);
	sl = search_list(list, len, x);
	mem_free(x);
	if (sl) return 1;

	dot = cast_uchar strchr(cast_const_char name, '.');
	if (!dot) return 0;
	x = stracpy(cast_uchar "*");
	add_to_strn(&x, dot);
	sl = search_list(list, len, x);
	mem_free(x);
	return sl;
}

int is_tld(unsigned char *name)
{
	char *end;
	unsigned long l;
	if (strlen(cast_const_char name) == 2 && upcase(name[0]) >= 'A' && upcase(name[0]) <= 'Z' && upcase(name[1]) >= 'A' && upcase(name[1]) <= 'Z' && casestrcmp(name, cast_uchar "gz"))
		return 1;
	l = strtoul(cast_const_char name, &end, 10);
	if (!*end && l <= 255)
		return 1;
	return search_list(domain_suffix, array_elements(domain_suffix), name);
}

int allow_cookie_domain(unsigned char *server, unsigned char *domain)
{
	int sl = (int)strlen(cast_const_char server);
	int dl = (int)strlen(cast_const_char domain);
	if (dl > sl) return 0;
	if (casestrcmp(domain, server + sl - dl)) return 0;
	if (dl == sl) return 1;
	if (!numeric_ip_address(server, NULL)) return 0;
#ifdef SUPPORT_IPV6
	if (!numeric_ipv6_address(server, NULL, NULL)) return 0;
#endif
	if (server[sl - dl - 1] != '.') return 0;
	if (search_list_and_wildcards(domain_suffix_x, array_elements(domain_suffix_x), domain)) return 1;
	if (!strchr(cast_const_char domain, '.')) return 0;
	if (search_list_and_wildcards(domain_suffix, array_elements(domain_suffix), domain)) return 0;
	return 1;
}
