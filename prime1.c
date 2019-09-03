#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

typedef unsigned long long int uint64_t;

#define BITMAP_SIZE(x) (((x)+7)/8)
#define GET_BIT(map, x) ( (map[x/8]) & (1<<(x%8)) )
#define SET_BIT(map, x) ( (map[x/8]) |= (1<<(x%8)) )
#define CLEAR_BIT(map, x) ( (map[x/8]) &= (~(1<<(x%8))) )

#define PRIME_COUNT 15 
#define PRIME_ARRAY_COUNT 49

unsigned char *primeBitmap = NULL;
uint64_t prime_array[PRIME_ARRAY_COUNT] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227};

unsigned char *create_bitmap(uint64_t max, uint64_t *mapSize)
{
	int size = 0;
	unsigned char *map = NULL;

	size = BITMAP_SIZE(max);
	map = (unsigned char*)malloc(size);
	memset(map, 0xFF, size);
	if( mapSize )
	{
		*mapSize = size;
	}

	return map;
}

uint64_t calculate_a_b_mod(uint64_t a, uint64_t b, uint64_t p)
{
	uint64_t result = 0;

	if( b == 1 )
	{
		return a;
	}
	result = calculate_a_b_mod(a, b>>1, p);
	result *= result;
	if( b & 0x1 )
	{
		result *= a;
	}

	return (result>p)?(result%p):result;
}

uint64_t calculate_a_b_mod_p(uint64_t a, uint64_t b, uint64_t p)
{
	uint64_t result = 0;

	result = calculate_a_b_mod(a%p, b, p);
	result %= p;

	return result;
}

int is_prime_fermat(uint64_t number)
{
	int i = 0;
	
	for( ; i < PRIME_COUNT; i++)
	{
		if( calculate_a_b_mod_p(prime_array[i], number-1, number) != 1 )
		{
			return 0;
		}
	}

	return 1;
}

int is_prime(uint64_t number)
{
	int i = 2;

	if( (number == 2) || (number == 3) )
	{
		return 1;
	}

	for( ; i <= number/2; i++)
	{
		if( number%i == 0 )
		{
			return 0;
		}
	}

	return 1;
}

uint64_t calculate_a_b(uint64_t a, uint64_t b)
{
	uint64_t result = 0;

	if( b == 1 )
	{
		return a;
	}
	result = calculate_a_b(a, b>>1);
	result *= result;
	if( b & 0x1 )
	{
		result *= a;
	}

	return result;
}

uint64_t get_prime(unsigned char *pMap, uint64_t max)
{
#define PRIME_CHECK_NORMAL 0
#define PRIME_CHECK_FERMAT 1
	uint64_t count = 0;
	uint64_t i = 2;
	uint64_t sum = 0;
	uint64_t result = 0;
	int flag = PRIME_CHECK_NORMAL;

	CLEAR_BIT(pMap, 0);
	CLEAR_BIT(pMap, 1);
	for( ; i <= max/2; i++)
	{
		if( i > prime_array[PRIME_COUNT-1] )
		{
			flag = PRIME_CHECK_FERMAT;
		}
		sum = i;
		if( GET_BIT(pMap, i) )
		{
			if( (flag == PRIME_CHECK_FERMAT) && is_prime_fermat(i) )
			{
				result ^= i;
				count++;

				sum += i;
				while( sum <= max )
				{
					CLEAR_BIT(pMap, sum);
					sum += i;
				}
			}
			else if( (flag == PRIME_CHECK_NORMAL) && is_prime(i) )
			{
				result ^= i;
				count++;

				sum += i;
				while( sum <= max )
				{
					CLEAR_BIT(pMap, sum);
					sum += i;
				}
			}
			else
			{
				CLEAR_BIT(pMap, i);
			}
		}
	}

	for( ; i <= max; i++)
	{
		if( GET_BIT(pMap, i) )
		{
			result ^= i;
			count++;
		}
	}

	printf("Result:%llu.\n", result);
	return count;
}

uint64_t calculate_xor(unsigned char *pMap, uint64_t max)
{
	uint64_t result = 0;
	uint64_t i = 0;

	for( ; i < max; i++)
	{
		if( GET_BIT(pMap, i) )
		{
			result ^= i;
		}
	}

	return result;
}

void dump_bitmap(unsigned char *pMap, uint64_t mapSize, const char *filename)
{
#define WRITE_STEP 4096 
	FILE *f = NULL;
	size_t remains = 0;
	int ret = 0;

	f = fopen(filename, "w+");

	remains = mapSize;
	ret = fwrite(pMap, remains, 1, f);
	if( !ret )
	{
		printf("Write file failed!\n");
	}

	if( f )
	{
		fclose(f);
	}
}

int main(int argc, char *argv[])
{
	uint64_t max = 0;
	uint64_t count = 0;
	uint64_t mapSize = 0;
	uint64_t result = 0;
	struct timeval t1;
	struct timeval t2;
	const char *filename = "prime_bitmap.data";

	if( argc < 2 )
	{
		printf("Usage:%s MAX\n", argv[0]);
		return 0;
	}
	max = atoll(argv[1]);
	primeBitmap = create_bitmap(max, &mapSize);
	printf("Bitmap size:%lluM\n", mapSize/1024/1024);

	gettimeofday(&t1, NULL);
	count = get_prime(primeBitmap, max);
	gettimeofday(&t2, NULL);
	printf("Prime count:%llu. Time consumed:%ld\n", count, t2.tv_sec-t1.tv_sec);

	dump_bitmap(primeBitmap, mapSize, filename);

	return 0;
}

