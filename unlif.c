/*
unlif: Extracts individual images from LIF files.
Copyright (C) 2014 Ian Martin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>   // for exit().
#include <string.h>
#include <inttypes.h> // for PRIx64.

// For open():
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 4096
#define bytes_per_pixel 2
#define image_size (image_width * image_height * bytes_per_pixel)
#define ctoi(c) ( (c) - '0' )
#define MIN(x, y)     ( ((x) < (y))? (x) : (y) )
#define PERMISSIONS   ( S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ) // aka 0644.
#define max_intensity 4095

const char* input_filename;
int input_fd;
int output_fd;
uint16_t image_width  = 1600;
uint16_t image_height = 1200;

const char needle[] = {'M', 0, 'e', 0, 'm', 0, 'B', 0, 'l', 0, 'o', 0, 'c', 0, 'k', 0, '_'};

char buf[BUF_SIZE];

uint64_t prev_offset = 0;
uint64_t data_offset = 0;

size_t buf_avail = 0;
size_t buf_index = 0;

char get_byte(void) {
	int result;
	
	if (buf_index >= buf_avail) {
		result = read(input_fd, buf, BUF_SIZE);
		if (result <= 0) {
			fprintf(stderr, "Unable to read data from input file at offset %" PRIu64 ": ", data_offset);
			perror(NULL);
			exit(5);
		}
		buf_avail = result;
		buf_index = 0;
	}
	
	data_offset++;
	
	assert(buf_index < buf_avail);
	assert(buf_index < BUF_SIZE);
	return buf[buf_index++];
}

uint16_t parse_four_digits(void) {
	uint16_t value = 0;
	uint8_t n;
	
	for (n = 0; n < 4; n++) {
		get_byte();
		value *= 10;
		value += ctoi(get_byte());
	}
	
	return value;
}

void write_all(const void* data, size_t len) {
	size_t progress = 0;
	ssize_t result;
	while (progress < len) {
		result = write(output_fd, (const char*)data + progress, len - progress);
		if (result > 0) {
			progress += result;
		} else {
			perror("Unable to write to output file");
			exit(6);
		}
	}
}

void splice(const char* filename, off64_t offset, off64_t length) {
	int fd;
	off64_t lseek_result, progress;
	ssize_t result;
	static char buffer[BUF_SIZE];

	fd = open(filename, O_RDONLY | O_LARGEFILE);
	if (fd < 0) {
		perror("Unable to open file");
		exit(3);
	} else {
		lseek_result = lseek64(fd, offset, SEEK_SET);
		if (lseek_result != offset) {
			fprintf(stderr, "Unable to seek to offset %" PRIu64 " of input file: ", offset);
			perror(NULL);
			exit(7);
		}

		for (progress = 0; progress < length; progress += result) {
			result = read(fd, buffer, MIN(length - progress, sizeof(buffer)));

			if (result <= 0) {
				fprintf(stderr, "Unexpected end of input file.\n");
				close(fd);
				exit(5);
			}

			write_all(buffer, result);
		}

		close(fd);
	}
}

static void found_image(uint16_t MemBlock, uint64_t offset, uint64_t size) {
	int num_images, index;
	char output_filename[sizeof("MemBlock_0000_000.pgm") * 2];
	char pgm_header[sizeof("P5\n00000 00000\n00000\n")];

	sprintf(pgm_header, "P5\n%u %u\n%u\n", image_width, image_height, max_intensity);

	num_images = size / image_size;
	for (index = 0; index < num_images; index++) {
		sprintf(output_filename, "MemBlock_%04u_%03u.pgm", MemBlock, index + 1);
		output_fd = open(output_filename, O_CREAT | O_TRUNC | O_WRONLY, PERMISSIONS);
		if (output_fd < 0) {
			perror("Unable to open output file");
			exit(4);
		} else {
			write_all(pgm_header, strlen(pgm_header));
			splice(input_filename, offset + index * image_size, image_size);
			close(output_fd);
			printf("Extracted \"%s\" from offset %" PRIu64 ".\n", output_filename, offset + index * image_size);
		}
	}
}

int main(int argc, char** argv) {
	ssize_t n, remaining;
	int match_progress = 0;
	int d;
	uint64_t delta;

	printf(
		"Unlif version 1.0\n"
		"Copyright (C) 2014 Ian Martin\n"
		"Unlif is free software and comes with ABSOLUTELY NO WARRANTY; not even for\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. For details, see LICENSE.\n"
		"\n"
	);

	if (argc >= 2) {
		input_filename = argv[1];
	} else {
		printf("Please specify lif filename, for example: %s file.lif\n", argv[0]);
		return 1;
	}

	input_fd = open(input_filename, O_RDONLY);
	if (input_fd < 0) {
		perror("Unable to open input file");
		return 2;
	}

	for(;;) {
		if (get_byte() == needle[match_progress]) {
			match_progress++;

			if (match_progress >= sizeof(needle)) {
				delta = data_offset - prev_offset;
				found_image(parse_four_digits(), data_offset + 2, delta);
				prev_offset = data_offset;
				match_progress = 0;
			}
		} else {
			match_progress = 0;
		}
	}
	
	return 0;
}
