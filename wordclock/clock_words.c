    /*
 * clock_words.c
 *
 *  Created on: Jun 21, 2015
 *      Author: robert.wassens
 */

#include <string.h>
#include <sys/types.h>
#include "c_types.h"
#include "font.h"
#include "clock_words.h"
#include "settings.h"

typedef enum {
	DIR_HORIZONTAL,
	DIR_VERTICAL
} TDIRECTION;

typedef struct {
	char wordText[10];
	/* TDIRECTION */ uint8_t dir;
	uint8_t row;
	uint8_t col;
	uint16_t bits;
} TKlocWord;

static const TKlocWord* _klockWords;	//this will point to the klocwords struct after init

static const TKlocWord _klockWords_9x8[] = {
		{"het", DIR_HORIZONTAL, 7, 0, 0b1},
		{"is", DIR_HORIZONTAL, 7, 2, 0b11},
		{"vijf_min", DIR_HORIZONTAL, 7, 6, 0b111},
		{"tien_min", DIR_HORIZONTAL, 6, 4, 0b1111},
		{"kwart", DIR_HORIZONTAL, 6, 0, 0b11111},
		{"voor", DIR_HORIZONTAL, 5, 4, 0b1111},
		{"over", DIR_HORIZONTAL, 5, 0, 0b1111},

		{"half", DIR_HORIZONTAL, 4, 0, 0b1111},

		{"een", DIR_HORIZONTAL, 0, 2, 0b111},
		{"twee", DIR_HORIZONTAL, 0, 0, 0b1111},
		{"drie", DIR_HORIZONTAL, 1, 3, 0b11011},
		{"vier", DIR_HORIZONTAL, 1, 0, 0b11101},
		{"vijf", DIR_HORIZONTAL, 4, 5, 0b111},
		{"zes", DIR_HORIZONTAL, 3, 0, 0b1011},
		{"zeven", DIR_HORIZONTAL, 3, 0, 0b10100111},
		{"acht", DIR_HORIZONTAL, 2, 0, 0b1111},
		{"negen", DIR_HORIZONTAL, 3, 1, 0b1101011},
		{"tien", DIR_HORIZONTAL, 1, 5, 0b1111},
		{"elf", DIR_VERTICAL, 2, 8, 0b111},
		{"twaalf", DIR_HORIZONTAL, 2, 3, 0b111111},	
		{"uur", DIR_HORIZONTAL, 0, 6, 0b111},	
		{"",0,0,0}
};

static const TKlocWord _klockWords_11x11[] = {
		{"het", DIR_HORIZONTAL, 10, 0, 0b111},
		{"is", DIR_HORIZONTAL, 10, 4, 0b11},
		{"vijf_min", DIR_HORIZONTAL, 8, 0, 0b1111},
		{"tien_min", DIR_HORIZONTAL, 9, 7, 0b1111},
		{"kwart", DIR_HORIZONTAL, 9, 3, 0b11111},
		{"half", DIR_HORIZONTAL, 7, 6, 0b1111},
		{"voor", DIR_HORIZONTAL, 8, 6, 0b1111},
		{"over", DIR_HORIZONTAL, 7, 0, 0b1111},


		{"een", DIR_HORIZONTAL, 5, 8, 0b111},
		{"twee", DIR_HORIZONTAL, 4, 0, 0b1111},
		{"drie", DIR_HORIZONTAL, 5, 0, 0b1111},
		{"vier", DIR_HORIZONTAL, 5, 4, 0b1111},
		{"vijf", DIR_HORIZONTAL, 2, 7, 0b1111},
		{"zes", DIR_HORIZONTAL, 6, 0, 0b111},
		{"zeven", DIR_HORIZONTAL, 3, 0, 0b11111},
		{"acht", DIR_HORIZONTAL, 4, 6, 0b1111},
		{"negen", DIR_HORIZONTAL, 3, 4, 0b11111},
		{"tien", DIR_HORIZONTAL, 2, 0, 0b1111},
		{"elf", DIR_HORIZONTAL, 2, 4, 0b111},
		{"twaalf", DIR_HORIZONTAL, 6, 3, 0b111111},	
		
		{"uur", DIR_HORIZONTAL, 1, 1, 0b111},	

		{"bijna", DIR_HORIZONTAL, 10, 7, 0b1111},	
		{"nu", DIR_HORIZONTAL, 9, 0, 0b11},	
		{"geweest", DIR_HORIZONTAL, 0, 2, 0b1111111},	


		{"wacht", DIR_HORIZONTAL, 4, 5, 0b11111},
		{"even", DIR_HORIZONTAL, 3, 1, 0b1111},

		{"by rmw", DIR_HORIZONTAL, 1, 5, 0b110111},
		
		{"he", DIR_HORIZONTAL, 10, 0, 0b11},
		{"lise", DIR_HORIZONTAL, 10, 3, 0b1111},
		{"en", DIR_HORIZONTAL, 9, 9, 0b11},
		{"ha", DIR_HORIZONTAL, 7, 3, 0b11},
		{"evi", DIR_HORIZONTAL, 5, 3, 0b111},


		{"am", DIR_HORIZONTAL, 0, 0, 0b1},
		{"pm", DIR_HORIZONTAL, 0, 10, 0b1},

		
		{"",0,0,0}
};
static const TKlocWord _klockWords_13x13_V2[] = {
        {"het", DIR_HORIZONTAL, 12, 0, 0b111},
        {"is", DIR_HORIZONTAL, 12, 4, 0b11},

        {"een_min", DIR_HORIZONTAL, 8, 2, 0b111},
        {"twee_min", DIR_HORIZONTAL, 8, 0, 0b1111},
        {"drie_min", DIR_HORIZONTAL, 10, 0, 0b1111},
        {"vier_min", DIR_HORIZONTAL, 11, 0, 0b1111},
        {"vijf_min", DIR_HORIZONTAL, 9, 9, 0b1111},
        {"zes_min", DIR_HORIZONTAL, 6, 0, 0b111},
        {"zeven_min", DIR_HORIZONTAL, 9, 0, 0b11111},
        {"acht_min", DIR_HORIZONTAL, 7, 0, 0b1111},
        {"negen_min", DIR_HORIZONTAL, 10, 4, 0b11111},
        {"tien_min", DIR_HORIZONTAL, 8, 9, 0b1111},

        {"elf_min", DIR_HORIZONTAL, 12, 10, 0b111},
        {"twaalf_min", DIR_HORIZONTAL, 6, 7, 0b111111},
        {"der_tnm", DIR_HORIZONTAL, 11, 6, 0b1111111},
        {"veer_tnm", DIR_HORIZONTAL, 8, 5, 0b11111111},
        {"kwart", DIR_HORIZONTAL, 7, 8, 0b11111},
        {"zes_tnm", DIR_HORIZONTAL, 6, 0, 0b1111111},
        {"zeven_tnm", DIR_HORIZONTAL, 9, 0, 0b111111111},
        {"acht_tnm", DIR_HORIZONTAL, 7, 0, 0b11111111},
        {"negen_tnm", DIR_HORIZONTAL, 10, 4, 0b111111111},

        {"half", DIR_HORIZONTAL, 4, 9, 0b1111},
        {"voor", DIR_HORIZONTAL, 5, 8, 0b1111},
        {"over", DIR_HORIZONTAL, 5, 3, 0b1111},

        {"een", DIR_HORIZONTAL, 2, 2, 0b111},
        {"twee", DIR_HORIZONTAL, 2, 0, 0b1111},
        {"drie", DIR_HORIZONTAL, 3, 9, 0b1111},
        {"vier", DIR_HORIZONTAL, 1, 0, 0b1111},
        {"vijf", DIR_HORIZONTAL, 3, 0, 0b1111},
        {"zes", DIR_HORIZONTAL, 2, 5, 0b111},
        {"zeven", DIR_HORIZONTAL, 2, 8, 0b11111},
        {"acht", DIR_HORIZONTAL, 3, 5, 0b1111},
        {"negen", DIR_HORIZONTAL, 1, 7, 0b11111},
        {"tien", DIR_HORIZONTAL, 1, 4, 0b1111},
        {"elf", DIR_HORIZONTAL, 0, 0, 0b111},
        {"twaalf", DIR_HORIZONTAL, 0, 3, 0b111111},

        {"uur", DIR_HORIZONTAL, 0, 10, 0b111},
        {"nu", DIR_HORIZONTAL, 12, 7, 0b11},

        {"wacht", DIR_HORIZONTAL, 3, 4, 0b11111},
        {"even", DIR_HORIZONTAL, 2, 8, 0b1111},

        {"by rmw", DIR_HORIZONTAL, 4, 0, 0b110000111},

        {"",0,0,0}
};

static const TKlocWord _klockWords_13x13[] = {
        {"het", DIR_HORIZONTAL, 12, 0, 0b111},
        {"is", DIR_HORIZONTAL, 12, 4, 0b11},

        {"een_min", DIR_HORIZONTAL, 8, 2, 0b111},
        {"twee_min", DIR_HORIZONTAL, 8, 0, 0b1111},
        {"drie_min", DIR_HORIZONTAL, 10, 0, 0b1111},
        {"vier_min", DIR_HORIZONTAL, 8, 5, 0b1111},
        {"vijf_min", DIR_HORIZONTAL, 9, 9, 0b1111},
        {"zes_min", DIR_HORIZONTAL, 6, 0, 0b111},
        {"zeven_min", DIR_HORIZONTAL, 9, 0, 0b11111},
        {"acht_min", DIR_HORIZONTAL, 7, 0, 0b1111},
        {"negen_min", DIR_HORIZONTAL, 10, 4, 0b11111},
        {"tien_min", DIR_HORIZONTAL, 9, 5, 0b1111},

        {"elf_min", DIR_HORIZONTAL, 12, 10, 0b111},
        {"twaalf_min", DIR_HORIZONTAL, 6, 7, 0b111111},
        {"der_tnm", DIR_HORIZONTAL, 11, 6, 0b1111111},
        {"veer_tnm", DIR_HORIZONTAL, 8, 5, 0b11111111},
        {"kwart", DIR_HORIZONTAL, 7, 8, 0b11111},
        {"zes_tnm", DIR_HORIZONTAL, 6, 0, 0b1111111},
        {"zeven_tnm", DIR_HORIZONTAL, 9, 0, 0b111111111},
        {"acht_tnm", DIR_HORIZONTAL, 7, 0, 0b11111111},
        {"negen_tnm", DIR_HORIZONTAL, 10, 4, 0b111111111},

        {"half", DIR_HORIZONTAL, 5, 9, 0b1111},
        {"voor", DIR_HORIZONTAL, 5, 4, 0b1111},
        {"over", DIR_HORIZONTAL, 5, 0, 0b1111},

        {"een", DIR_HORIZONTAL, 3, 2, 0b111},
        {"twee", DIR_HORIZONTAL, 3, 0, 0b1111},
        {"drie", DIR_HORIZONTAL, 4, 9, 0b1111},
        {"vier", DIR_HORIZONTAL, 1, 1, 0b1111},
        {"vijf", DIR_HORIZONTAL, 4, 0, 0b1111},
        {"zes", DIR_HORIZONTAL, 3, 5, 0b111},
        {"zeven", DIR_HORIZONTAL, 3, 8, 0b11111},
        {"acht", DIR_HORIZONTAL, 4, 5, 0b1111},
        {"negen", DIR_HORIZONTAL, 1, 5, 0b11111},
        {"tien", DIR_HORIZONTAL, 2, 0, 0b1111},
        {"elf", DIR_HORIZONTAL, 2, 10, 0b111},
        {"twaalf", DIR_HORIZONTAL, 2, 4, 0b111111},

        {"uur", DIR_HORIZONTAL, 0, 1, 0b111},

        {"bijna", DIR_HORIZONTAL, 11, 0, 0b11111},
        {"nu", DIR_HORIZONTAL, 12, 7, 0b11},
        {"geweest", DIR_HORIZONTAL, 0, 5, 0b1111111},

        {"wacht", DIR_HORIZONTAL, 4, 4, 0b11111},
        {"even", DIR_HORIZONTAL, 3, 9, 0b1111},

        {"by rmw", DIR_HORIZONTAL, 1, 10, 0b111},

        {"",0,0,0}
};

const char* _hourNames[] = {
		"twaalf",
		"een",
		"twee",
		"drie",
		"vier",
		"vijf",
		"zes",
		"zeven",
		"acht",
		"negen",
		"tien",
		"elf",
		"twaalf"
};

const char* _minutesNames[] = {
		"by rmw", //index 0 is not used, but temperely use am
		"een_min",
		"twee_min",
		"drie_min",
		"vier_min",
		"vijf_min",
		"zes_min",
		"zeven_min",
		"acht_min",
		"negen_min",
		"tien_min",
		"elf_min",
		"twaalf_min",
		"der_tnm",
		"veer_tnm",
		"kwart",
		"zes_tnm",
		"zeven_tnm",
		"acht_tnm",
		"negen_tnm"
};

static const uint8_t _brightness = 80;


void CWSet(const char* word, uint8_t r, uint8_t g, uint8_t b) {
	uint32_t idx = 0;
	bool found = FALSE;
	bool done = FALSE;
	
	for(idx = 0; !done; idx++) {
		if (strcmp(word, _klockWords[idx].wordText) == 0) {
			found = TRUE;
			done = TRUE;
		} else if (_klockWords[idx].wordText[0] == '\0') {
			done = TRUE;
		}
	}
	
	
	if (found) {
		idx--;
		FontLine(_klockWords[idx].row, _klockWords[idx].col, _klockWords[idx].bits, 
				_klockWords[idx].dir == DIR_HORIZONTAL, r, g, b);
	}
}


void CWDisplayTime(uint32_t hours, uint32_t minutes, uint8_t r, uint8_t g, uint8_t b) {
	
	CWSet("het", r, g, b);
	CWSet("is", r, g, b);
	
	// Round the minutes to a multiple og five
	int t = minutes % 5;
	if (t <= 2) {
		minutes -= t;
		if (t == 0) {
			CWSet("nu", r, g, b);
		} else {
			CWSet("geweest", r, g, b);
		}
	} else {
		minutes += 5 - t;
		CWSet("bijna", r, g, b);
		if (minutes == 60){
			minutes = 0;
			hours += 1;
			hours %= 12;
		}
	}

	// Display the current hour up to quarter past... After that show the next hour
	if (minutes > 15) {
		hours += 1;
	}

	hours %= 12;
	CWSet(_hourNames[hours], r, g, b);

	switch (minutes) {
	case 0:
		CWSet("uur", r, g, b);
		break;

	case 30:
		CWSet("half", r, g, b);
		break;

	case 15:
	case 45:
		CWSet("kwart", r, g, b);
		break;

	case 25:
	case 35:
		CWSet("half", r, g, b);
	case 5:
	case 55:
		CWSet("vijf_min", r, g, b);
		break;
		
	case 20:
	case 40: 
		CWSet("half", r, g, b);
	case 10:
	case 50:
		CWSet("tien_min", r, g, b);
		break;
	}
	
	if ((minutes > 0 && minutes <= 15) ||
		(minutes > 30 && minutes <= 40)){
		CWSet("over", r, g, b);
	} 
	if ((minutes > 15 && minutes < 30) ||
		(minutes >= 45 && minutes <= 55)){
		CWSet("voor", r, g, b);
	}
}

void CWDisplayAccurateTime(uint32_t hours, uint32_t minutes,  uint32_t seconds, uint8_t r, uint8_t g, uint8_t b)
{

	CWSet("het", r, g, b);
	CWSet("is", r, g, b);


	// dit werkt wel maar wordt erg druk
	/*
	if (seconds > 50 && seconds <= 95) {
		minutes += 1;
		CWSet("bijna", r, g, b);
	} else if (seconds < 50 && seconds >= 10) {
		CWSet("geweest", r, g, b);
	} else {
		CWSet("nu", r, g, b);
	}
    */
	// Display the current hour up to quarter past... After that show the next hour
	if (minutes > 19) {
		hours += 1;
	}
	hours %= 12;
	CWSet(_hourNames[hours], r, g, b);


	if (minutes == 0) {
		CWSet("uur", r, g, b);
	}
	if (minutes >= 1 && minutes <= 19) {
		CWSet(_minutesNames[minutes], r, g, b);
		CWSet("over", r, g, b);
	}
	if (minutes >= 20 && minutes <= 29) {
		CWSet(_minutesNames[10 - (minutes % 10)], r, g, b);
		CWSet("voor", r, g, b);
		CWSet("half", r, g, b);
	}
	if (minutes == 30) {
		CWSet("half", r, g, b);
	}
	if (minutes >= 31 && minutes <= 40) {
		CWSet(_minutesNames[minutes % 15], r, g, b);
		CWSet("over", r, g, b);
		CWSet("half", r, g, b);
	}
	if (minutes == 45) {
		CWSet("kwart", r, g, b);
		CWSet("voor", r, g, b);
	}
	if (minutes >=41 && minutes <= 44) {
		//12:41 == 19 voor 1
		CWSet(_minutesNames[20 - (minutes % 10)], r, g, b);
		CWSet("voor", r, g, b);
	}
	if (minutes >=46 && minutes <= 59) {
		//12:48 == 12 voor 1
		CWSet(_minutesNames[15 - (minutes % 15)], r, g, b);
		CWSet("voor", r, g, b);
	}
}

void CWInit() {
	
	switch (g_settings.hardwareType) {
		case HARDWARE_9_8:
			_klockWords = _klockWords_9x8;
		break;
		default:
		case HARDWARE_11_11:
			_klockWords = _klockWords_11x11;
		break;
		case HARDWARE_13_13:
		case HARDWARE_13_13_NOT_ACCURATE:
			_klockWords = _klockWords_13x13;
		break;
		case HARDWARE_13_13_V2:
			_klockWords = _klockWords_13x13_V2;
		break;
	}
}
