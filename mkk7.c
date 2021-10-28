#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>

// location of the BASIC program first instruction
#define TXTTAB 0x49FC

// room reserved for the CALL instruction
#define TRAMPOLINE_ROOM_RESERVE 16

// default binary load address
#define DEFAULT_BIN_LOAD_ADDRESS (TXTTAB + TRAMPOLINE_ROOM_RESERVE)

static uint16_t load_address = DEFAULT_BIN_LOAD_ADDRESS;
static uint16_t binary_entry = 0;
static char *file_name = "VG5000";
static FILE *file_in;
static FILE *file_out;

struct __attribute__((__packed__)) k7_file_header {
	char type; // 'M' binary
	char name[6];
	char version; // 0
	char start_line[5];
	char prot; // prevents basic interpreter from running
	uint16_t csum_workspace; // 0
	uint16_t start_address; // load address
	uint16_t length;
	uint16_t csum;
};

struct __attribute__((__packed__)) basic_instruction {
	uint16_t next_ptr;
	uint16_t line_num;
	char instruction;
	char params[8];
};

static uint16_t to_little(uint16_t v) {
	uint16_t l;
	((unsigned char *)&l)[0] = v;
	((unsigned char *)&l)[1] = v >> 8;
	return l;
}

static uint16_t from_little(uint16_t l) {
	return ((unsigned char *)&l)[0] | ((unsigned char *)&l)[1] << 8;
}

static void repeat_byte(char b, int times) {
	int i;
	for (i = 0; i < times; i++)
		putc(b, file_out);
}

static void append_header(int *data_length_offset, int *csum_offset) {
	struct k7_file_header header = {
		.type = 'M',
		.version = 0,
		.start_line = "10",
		.prot = 0,
		.csum_workspace = 0,
	};

	strncpy(header.name, file_name, sizeof(header.name));
	header.start_address = to_little(TXTTAB);

	repeat_byte(0xD3, 10);
	fwrite(&header, sizeof(header), 1, file_out);
	repeat_byte(0xD6, 10);

	*data_length_offset = 10 + offsetof(struct k7_file_header, length);
	*csum_offset = 10 + offsetof(struct k7_file_header, csum);
}

static void append_data(int *data_length, int *csum) {
	long data_offset = ftell(file_out);
	struct basic_instruction inst;
	int padding;
	int c;

	int params_len = snprintf(inst.params, sizeof(inst.params), " %d", binary_entry);
	if (params_len >= sizeof(inst.params))
		fprintf(stderr, "warning: trampoline overflow\n");

	params_len ++; // include terminating nul

	inst.next_ptr = to_little(TXTTAB + 5 + params_len);
	inst.line_num = to_little(10);
	inst.instruction = 0x9F;

	fwrite(&inst, 5 + params_len, 1, file_out);
	fwrite("\0", 2, 1, file_out); // end of trampoline program
	
	padding = load_address - TXTTAB - (ftell(file_out) - data_offset);

	if (padding < 0) {
		fprintf(stderr, "error: load_address too low\n");
		exit(1);
	}

	repeat_byte(0, padding);

	while ((c = fgetc(file_in)) != EOF)
		fwrite(&c, 1, 1, file_out);

	// align on 16 bits for the csum
	while ((*data_length = ftell(file_out) - data_offset) % 2)
		fwrite("", 1, 1, file_out);

	*csum = 0;
	fseek(file_out, data_offset, SEEK_SET);

	while (!feof(file_out)) {
		unsigned char v;
		if (fread(&v, sizeof(v), 1, file_out))
			*csum += v;
	}

	repeat_byte(0, 10);
}

static void fix_header(int data_length_offset, int data_length, int csum_offset, int csum) {
	uint16_t v = to_little(data_length);

	fseek(file_out, data_length_offset, SEEK_SET);
	fwrite(&v, sizeof(v), 1, file_out);

	v = to_little(csum);

	fseek(file_out, csum_offset, SEEK_SET);
	fwrite(&v, sizeof(v), 1, file_out);
}

static void usage() {
	fprintf(stderr, "mkk7 [-l <load_addr>] [-e <entry>] [-n <name>] <binary> <k7>\n");
	fprintf(stderr, "\t-l <load_addr>\tload address [0x%04X]\n", (unsigned int)DEFAULT_BIN_LOAD_ADDRESS);
	fprintf(stderr, "\t-e <entry>\tabsolute address to jump to [<load_addr>]\n");
	fprintf(stderr, "\t-n <name>\tfile name [VG5000]\n");
}

int main(int argc, char **argv) {
	int rc;

	while (-1 != (rc = getopt(argc, argv, "e:l:n:"))) {
		switch (rc) {
			case 'e':
				binary_entry = strtoul(optarg, NULL, 0);
				break;
			case 'l':
				load_address = strtoul(optarg, NULL, 0);
				break;
			case 'n':
				file_name = optarg;
				break;
			default:
				usage();
				exit(1);
		}
	}

	if (binary_entry == 0)
		binary_entry = load_address;

	if (optind + 2 > argc) {
		usage();
		exit(1);
	}

	file_in = fopen(argv[optind], "r");
	if (!file_in) {
		fprintf(stderr, "Unable to open %s for reading\n", argv[optind]);
		exit(1);
	}

	file_out = fopen(argv[optind + 1], "w+");
	if (!file_out) {
		fprintf(stderr, "Unable to open %s for writing\n", argv[optind + 1]);
		exit(1);
	}

	int data_length_offset, data_length;
	int csum_offset, csum;
	append_header(&data_length_offset, &csum_offset);
	append_data(&data_length, &csum);
	fix_header(data_length_offset, data_length, csum_offset, csum);

	fclose(file_in);
	fclose(file_out);

	return 0;
}
