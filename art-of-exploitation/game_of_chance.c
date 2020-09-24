#include "hacking.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define DATAFILE "/var/chance.data" // File to store user data

// Custom user struct to store data about users
struct user {
	int uid;
	int credits;
	int highscore;
	char name[100];
	int (*current_game)();
};

int get_player_data();
void register_new_player();
void update_player_data();
void show_highscore();
void jackpot();
void input_name();
void print_cards(char *, char *, int);
int take_wager(int, int);
void play_the_game();
int pick_a_number();
int dealer_no_match();
int find_the_ace();
void fatal(char *);

// Global variable
struct user player; // Player struct

int main(void)
{
	int choice, last_game;

	srand(time(0));

	if (get_player_data() == -1)
		register_new_player();

	while (choice != 7) {
		printf("-=[ Game of Chance Menu ]=-\n");
		printf("1 - Play the Pick a Number game\n");
		printf("2 - Play the No Match Dealer game\n");
		printf("3 - Play the Find the Ace game\n");
		printf("4 - View current high score\n");
		printf("5 - Change your user name\n");
		printf("6 - Reset your account at 100 credits\n");
		printf("7 - Quit\n");
		printf("[Name: %s]\n", player.name);
		printf("[You have %u credits]  -> ", player.credits);
		scanf("%d", &choice);

		if ((choice < 1) || (choice > 7))
			printf("\n[!!] The number %d is an invalid selection.\n\n", choice);
		else if (choice < 4) {
			if (choice != last_game) {
				if (choice == 1)
					player.current_game = pick_a_number;
				else if (choice == 2)
					player.current_game = dealer_no_match;
				else
					player.current_game = find_the_ace;
				last_game = choice;
			}
			play_the_game();
		} else if (choice == 4)
			show_highscore();
		else if (choice == 5) {
			printf("\nChange user name\n");
			printf("Enter your new name: ");
			input_name();
			printf("Your name has been changed.\n\n");
		} else if (choice == 6) {
			printf("\nYour account has been reset with 100 credits.\n\n");
			player.credits = 100;
		}
	}
	update_player_data();
	printf("\nThanks for playing!\n");
	return 0;
}

// This function readcs the player data for the current uid
// from the file. It returns -1 if it is unable to find player
// data for the current uid.
int get_player_data()
{
	int fd, uid, read_bytes;
	struct user entry;

	uid = getuid();

	fd = open(DATAFILE, O_RDONLY);
	if (fd == -1)
		return -1;
	read_bytes = read(fd, &entry, sizeof(struct user)); // Read the first chunk
	while (entry.uid != uid && read_bytes > 0) { // Loop until uid is found
		read_bytes =
			read(fd, &entry, sizeof(struct user)); // Keep reading to entry
	}
	close(fd);
	if (read_bytes < sizeof(struct user)) // This means that the EOF was reached
		return -1;
	else
		player = entry;
	return 1;
}

void register_new_player()
{
	int fd;

	printf("-=-={ New Player Registration }=-=-");
	printf("Enter your name: ");
	input_name();

	player.uid = getuid();
	player.highscore = player.credits = 100;
	fd = open(DATAFILE, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (fd == -1)
		fatal("in register_new_player() while opening file");
	write(fd, &player, sizeof(struct user));
	close(fd);

	printf("\nWelcome to the Fame of Chance %s.\n", player.name);
	printf("You have been given %u credits.\n", player.credits);
}

// This function writes the current player data to the file.
// It is used primarily for updating the credits after games.
void update_player_data()
{
	int fd, i, read_uid;
	char burned_byte;

	fd = open(DATAFILE, O_RDWR);
	if (fd == -1)
		fatal("in update_player_data() while opening file");
	read(fd, &read_uid, 4); // Read uid from the first struct
	while (read_uid != player.uid) {
		for (int i = 0; i < sizeof(struct user) - 4; i++) { // Read through the
			read(fd, &burned_byte, 1); // rest of the struct
		}
		read(fd, &read_uid, 4); // Read UID from the next struct
	}
	write(fd, &(player.credits), 4);   // Update credits
	write(fd, &(player.highscore), 4); // Update highscore
	write(fd, &(player.name), 100);	   // Update name
	close(fd);
}

void show_highscore()
{
	unsigned int top_score = 0;
	char top_name[100];
	struct user entry;
	int fd;

	printf("\n==============| HIGH SCORE |==============\n");
	fd = open(DATAFILE, O_RDONLY);
	if (fd == -1)
		fatal("in show_highscore() while opening file");
	while (read(fd, &entry, sizeof(struct user)) > 0) {
		if (entry.highscore > top_score) {
			top_score = entry.highscore;
			strcpy(top_score, entry.name);
		}
	}
	close(fd);
	if (top_score > player.highscore)
		printf("%s has the high score of %u\n", top_name, top_score);
	else
		printf("You currently have the high scoure of %u credits!\n",
			   player.highscore);
	printf("==========================================\n");
}

void jackpot()
{
	printf("*+*+* JACKPOT *+*+*\n");
	printf("You have won the jackpot of 100 credits!\n");
	player.credits += 100;
}

// This function is used to input player name, since scanf("%s", &whatever)
// will stop input at the first space
void input_name()
{
	char *name_ptr, input_char = '\n';
	while (input_char == '\n')	  // Flush any leftover
		scanf("%c", &input_char); // newline chars.

	name_ptr = (char *)&(player.name); // name_ptr = player names address
	while (input_char != '\n') {	   // Loop until newline
		*name_ptr = input_char;		   // Put the input char into name field
		scanf("%c", &input_char);	   // Get next char
		name_ptr++;					   // Increment pointer
	}
	*name_ptr = 0;
}

// This function prints the 3 cards for the Find the Ace game.
// It expects a message to display, a pointer to the cards array
// and the card the user has picked as input. If the user_pick is
// -1, then the selection numbers are displayed.
void print_cards(char *message, char *cards, int user_pick)
{
	int i;

	printf("\n\t*** %s ***\n", message);
	printf("\t._.\t._.\t._.\t._.\t._.\n");
	printf("Cards:\t|%c|\t|%c|\t|%c|\n\t", cards[0], cards[1], cards[2]);
	if (user_pick == -1) {
		printf("1 \t 2 \t 3 \t\n");
	} else {
		for (i = 0; i < user_pick; i++) {
			printf("\t");
		}
		printf(" ^-- your pick\n");
	}
}

// This function inputs wagers for both the No Match Dealer and Find the Ace
// games. It expects the avaliable credits and the previous wager as arguments
// The previous_wager is only important for the second wasger in the
// Find the Ace games. The funtion returns -1 if the wager is too big or too
// little, and it returns the wager amount otherwise.
int take_wager(int avaliable_credits, int previous_wager)
{
	int wager, total_wager;

	printf("How many of your %d credits would you like to wager?",
		   avaliable_credits);
	scanf("%d", &wager);
	if (wager < 1) {
		printf("Nice try, but you must wager a positive number!\n");
		return -1;
	}
	total_wager = previous_wager + wager;
	if (total_wager > avaliable_credits) {
		printf("Your total wager of %d is more than you have!\n", total_wager);
		printf("You only have %d avaliable credits, try again.\n",
			   avaliable_credits);
		return -1;
	}
	return wager;
}
