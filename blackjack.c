#pragma warning (disable:4996)

/* BlackJack project prepared by Pavel Arefyev for C Programming course
Metropolia University of Applied Sciences, Smart IoT Systems major, 2nd year
...
See more regulations and mannual in pdf file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "blackjack.h"

int main(void)
{
	srand(time(NULL)); // Genearting random seed  
	int selection;
	FILE *data; // Global statistics file
	FILE *log;
	char buffer[MAX_LENGTH]; // Used for any kind of reading  ч
	
	// Initializing structure of constant limits
	const constValues limit = { 
		.maxLength = MAX_LENGTH, .valueNum = VALUE_NUM, .suiteNum = SUITE_NUM, 
		.value = { "A", "J", "2", "3", "4", "5", "6",
					"7", "8", "9", "10", "Q", "K"}, 
		.suite = { "Spades", "Hearts", "Clubs", "Diamonds" } 
	};
	stats global = { 0 };

	// List header pointer, initially null
	card *poolDealerHead = NULL;
	card *poolPlayerHead = NULL;

	// Read global stats from data file
	data = fopen(DAT_FILE, "rb");
	if (!readStats(data, &global)) {
		stats global = { 0 };
	}
	else if (data != NULL ) fclose(data);

	// Menu loop        
	while(!readOptionNumber(buffer, MAX_LENGTH, &selection));
	while (selection != 0) {
		switch (selection) {
			// User started the game
			case 1:
				log = fopen(LOG_FILE, "w");

				game(log, &poolDealerHead, &poolPlayerHead, &global, limit, buffer);

				// Write global stats to dat file
				data = fopen(DAT_FILE, "wb");
				fwrite(&global, sizeof(stats), 1, data);
				fclose(data);

				break;

			case 2:
				// Check if the file contains any info
				log = fopen(LOG_FILE, "r");
				if (viewLog(log, buffer, MAX_LENGTH) < 1) {
					printf("\n No previous games recorded.\n");
				}
				pressEnter(buffer, MAX_LENGTH);

				break;

			case 3:
				showStats(global);
				pressEnter(buffer, MAX_LENGTH);

				break;

			case 4:
				showAchievements(global);
				pressEnter(buffer, MAX_LENGTH);

				break;
		}

		while (!readOptionNumber(buffer, MAX_LENGTH, &selection));
	}

	printf("\n Thanks for playing in my casino!\n");  

	return 0;
}

// Initial "raw" reading of the userInput line.
char *readLine(char *userInput, const int size)
{
	if (fgets(userInput, size, stdin) == NULL) {
		printf("\n Error occured. 0: Unknown error.\n");
		strcpy(userInput, "undef");
	}  

	else if (userInput[strlen(userInput) - 1] != '\n') {
		printf("\n Error occured. 1: Input exceedes the maximum number of characters (%d).\n", size-2);
		while (getchar() != '\n'); // Clear stdin buffer.
		strcpy(userInput, "undef");
	}
	
	// Removing tailer new line.
	userInput[strlen(userInput) - 1] = '\0';
	return userInput;
}

// Read number required by the menu selection
bool readOptionNumber(char *userInput, const int size, int *selection)
{
	bool result = true;

	// Check for errors in "raw" reading
	printMenu();
	printf(" Choose an option from the menu (0-4): ");
	userInput = readLine(userInput, size);

	// Read a number in specific range
	if (sscanf(userInput, "%d", &*selection) != 1 || *selection < 0 || *selection > 4) {
		printf("\n Error occured. 2: Incorrect menu input. Enter a single number (0-4).\n");
		result = false;
	}

	return result;
}

// User selection menu output
void printMenu(void)
{
	printf("\n\n\n\n\t\tBLACK JACK V1 by Pavel Arefyev\n\n\n\n");
	printf("\n 1) Start the game\n");
	printf(" 2) View previous game log\n");
	printf(" 3) View global statistics\n");
	printf(" 4) Achievements\n");
	printf(" 0) Exit\n");
}

/* Target list - the one where we add the card
Second - the other pool for second comparison
The process of dealing the card is separated into multiple steps:
1. Generating the random possible card
2. Checking if the card is already on the lists of drawn cards
3. Adding the card to the drawn list pool (player's pool)
4. Return the card pointer to the added element*/
card *dealCard(card **target, card **second, const constValues limit) {
	bool copy = true;
	card *nextCard = NULL;
	
	// Generating card and checking if it is already in player's pool
	while (copy) {
		nextCard = generateCard(limit);
		if (nextCard == NULL) {
			break;
		}
		copy = false;

		// Duplicates checking loop in target pool
		while (*target != NULL) {
			if (strcmp((*target)->suite, nextCard->suite) == 0 && strcmp((*target)->value, nextCard->value) == 0) {
				copy = true;
			}

			target = &(*target)->next;
		}

		// Duplicates checking loop in another pool
		while (*second != NULL) {
			if (strcmp((*second)->suite, nextCard->suite) == 0 && strcmp((*second)->value, nextCard->value) == 0) {
				copy = true;
			}

			second = &(*second)->next;
		}

		if (copy) free(nextCard); // if the card is a copy free unused memory for further generation
	}

	// Adding an element to the list, cheking that card generated correctly
	if (nextCard == NULL) {
		printf("\n Error occured. 3: Technical error in memory allocation.\n");
	}
	else {
		*target = nextCard;
		nextCard->next = NULL; // if first card, then points to NULL
	}

	return nextCard;
}

// Generating card and returning a pointer to the generated node
card *generateCard(const constValues limit) {
	card* generatedCardPointer;
	generatedCardPointer = (card*)malloc(sizeof(card));
	if (generatedCardPointer == NULL) {
		return generatedCardPointer; // return in case of memory allocation error
	}

	int number;

	// Generating card rank
	number = rand() % limit.valueNum;
	strcpy(generatedCardPointer->value, limit.value[number]);
	
	// Generating card suite
	number = rand() % limit.suiteNum;
	strcpy(generatedCardPointer->suite, limit.suite[number]);

	generatedCardPointer->aceOne = false;
	
	//generatedCardPointer = &generatedCard;
	return generatedCardPointer;
}

// Empty the card pool linked list. Used at the end of the round.
void empty(card **ppn)
{
	card *prev = *ppn;

	while (*ppn != NULL) {
		*ppn = (*ppn)->next;
		free(prev);
		prev = *ppn;
	}
}

// Print the cards in the pool
void printCurrentCards(card **dealer, card **player)
{
	int count = 0;
	int sum = 0;

	// Print player cards
	printf("\n\n Player cards:\n");
	while (*player != NULL) {
		count++;
		printf(" Card %d: %10s of %s\n", count, (*player)->value, (*player)->suite);
		player = &(*player)->next;
	}

	count = 0;

	// Print dealer's cards. Only one card is dealt as in European casinos.
	printf("\n Dealer cards:\n");
	while (*dealer != NULL) {
		count++;
		printf(" Card %d: %10s of %s\n", count, (*dealer)->value, (*dealer)->suite);
		dealer = &(*dealer)->next;
	}
}

// Gaming process
int game(FILE *log, card **dealer, card **player, stats *global, const constValues limit, char *userInput)
{
	int sum = 0; // Total player's hand value
	int dealerSum = 0; // Total dealer's table value
	int counter = 2; // For statistics
	bool bjDealer = false; // Flag: dealer got blackjack
	bool bjPlayer = false; // Flag: player got blackjack
	bool noDealerTurn = false; // Flag: skip dealer turn
	bool quit = false; // Flag: player quitted after the deal
	card *currentCard;
	strcpy(userInput, "undef"); // Setting the choice to "not chosen yet"

	// Beginning of the game
	printf("\n%10s\n", " Dealing the cards...");

	// Dealing the cards to the player
	currentCard = dealCard(player, dealer, limit);
	currentCard = dealCard(player, dealer, limit);
	sum = calculateValue(player);

	// Dealing the card to dealer
	currentCard = dealCard(dealer, player, limit);
	dealerSum = calculateValue(dealer);

	printCurrentCards(dealer, player);

	// Player got BlackJack
	if (sum == 21) {
		printf(" That's a BlackJack!\n");
		bjPlayer = true;
		strcpy(userInput, "stand");
		global->blackjack++;
	}

	/* Dealer drew ace as the first card. If player didn't get blackjack
	skip this condition, as the outcome is the same */
	else if (dealerSum == 11) {
		while (strcmp(userInput, "yes") != 0 && strcmp(userInput, "no") != 0) {
			printf("\n Dealer got Ace. Would you like to take your bet back and quit? (yes/no): ");
			userInput = readLine(userInput, limit.maxLength);
		}
		if (strcmp(userInput, "yes") == 0) {
			strcpy(userInput, "stand");
			noDealerTurn = true;
			quit = true;
		}
	}

	// Player's turn
	while (strcmp(userInput, "stand") != 0 && strcmp(userInput, "hit") != 0) {
		printf("\n Ask more cards? (hit/stand): ");
		userInput = readLine(userInput, limit.maxLength);
	}
	while (strcmp(userInput, "stand") != 0) {
		// Deal the card
		currentCard = dealCard(player, dealer, limit);
		sum = calculateValue(player);
		printCurrentCards(dealer, player);

		if (sum > 21) {
			printf("\n That's a bust!\n");
			noDealerTurn = true;
			global->bust++;
			break;
		}
		
		counter++;

		// Wait for the player's response 
		strcpy(userInput, "undef");
		while (strcmp(userInput, "stand") != 0 && strcmp(userInput, "hit") != 0) {
			printf("\n Ask more cards? (hit/stand): ");
			userInput = readLine(userInput, limit.maxLength);
		}

	}

	// Dealer's turn
	printf(" \n Dealer's turn...");
	while (dealerSum < 17 && noDealerTurn == false) {
		currentCard = dealCard(dealer, player, limit);
		dealerSum = calculateValue(dealer);

		printCurrentCards(dealer, player);

		pressEnter(userInput, limit.maxLength);
	}

	// Turns ended. Checking the winner.
	// Both got blackjack
	if (quit == true) {
		printf("\n Bet withdrawn. \n");
		global->quit++;
	}

	else if (bjPlayer && bjDealer) {
		printf("\n BlackJack push! You just got 1 in 2500 chance!\n");
		global->bjPush++;
	}

	// Player lost
	else if (bjDealer || sum > 21 || (dealerSum > sum && dealerSum <= 21)) {
		printf("\n You lost. Better luck next time.\n");
		global->lose++;
	}

	// Player won the casino
	else if (bjPlayer || dealerSum > 21 || (sum > dealerSum && sum <= 21)) {
		printf("\n You won the casino!\n");
		global->win++;
	}
			
	// Push
	else {
		printf("\n That's a push.\n");
		global->push++;
	}

	printf(" Saving log...\n");
	if (!writeLog(log, dealer, player, sum, dealerSum)) printf(" Error 4: Unable to write log.\n");
	printf(" Press enter to proceed to the main menu...\n");
	pressEnter(userInput, limit.maxLength);

	// Update statistics
	if (counter > global->maxCards) {
		global->maxCards = counter;
	}
	global->games++;

	// Empty lists i.e. put the cards back in the deck
	empty(dealer);
	empty(player);

	return 0;
}

/* Calculate current sum based on ranks
aceOne boolean is true, when Ace rank equals to 1 */
int calculateValue(card **ppn) 
{
	int sum = 0;
	bool ace = false; // Flag: ACE is on 
	card *head = *ppn;

	// Calculating the total sum of the pool
	while (*ppn != NULL) {
		if (strcmp((*ppn)->value, "K") == 0 || strcmp((*ppn)->value, "Q") == 0 || strcmp((*ppn)->value, "J") == 0) {
			sum += 10;
		}

		// Drawing ace with aceOne = false by default
		else if (strcmp((*ppn)->value, "A") == 0) {
			sum += 11;
			ace = true;
		}

		// Drawing a number-value card
		else {
			sum += atoi((*ppn)->value);
		}

		ppn = &(*ppn)->next;
	}

	ppn = &head;

	// Dynamically adjust ace value (find an ace in the pool and lower the sum)
	if (ace == true && sum > 21 && *ppn != NULL) {
		while ((*ppn)->aceOne != false && strcmp((*ppn)->value, "A") == 0) {
			ppn = &(*ppn)->next;
		}
		sum -= 10;
		(*ppn)->aceOne = true;
	}

	return sum;
}

/* Used characters array insted of getchar()
in order to avoid buffer overflow */
void pressEnter(char *buffer, int size)
{
	buffer = readLine(buffer, size);
}

// Write the main points of the previous game to log file
bool writeLog(FILE *log, card **dealer, card **player, int sumPlayer, int sumDealer)
{
	if (log == NULL) {
		return false;
	}

	int count = 0;

	// Write player's cards to log
	fprintf(log ,"\n Previous game log.\n\n");
	fprintf(log, " Drawn cards.\n\n");
	fprintf(log, " Player has drawn:\n");
	while (*player != NULL) {
		count++;
		fprintf(log, " Card %d: %13s of %s\n", count, (*player)->value, (*player)->suite);
		player = &(*player)->next;
	}

	count = 0;

	// Write dealer's cards to log
	fprintf(log, "\n Player has drawn:\n");
	while (*dealer != NULL) {
		count++;
		fprintf(log, " Card %d: %13s of %s\n", count, (*dealer)->value, (*dealer)->suite);
		dealer = &(*dealer)->next;
	}

	// Print the sum
	// NOTE: The sum is not avaliable while playing, only in log
	fprintf(log, "\n Dealer sum:%10d\n", sumDealer);
	fprintf(log, " Player sum:%10d\n\n", sumPlayer);

	fclose(log);
	return true;
}

// Print the content of LOG file to the screen
int viewLog(FILE *log, char *line, const int size)
{
	int lc = 0; // Line counter

	if (log == NULL) {
		return 0;
	}

	// Read log
	while (!feof(log)) {
		if (fgets(line, size, log) != NULL) {
			lc++;
			printf("%s", line);
		}
	}

	printf(" Press enter to proceed to the main menu...\n");

	fclose(log);
	return lc;
}

// Read statistics in binary mode
bool readStats(FILE *data, stats *global)
{
	if (data == NULL) {
		return false;
	}

	else if (fread(global, sizeof(stats), 1, data) != 1) {
		return false;
	}

	return true;
}

// Display statistics table
void showStats(const stats global)
{
	float winrate = (float)global.win / global.games;

	printf("\n Statistics:\n");
	printf(" Total games played%21d\n", global.games);
	printf(" Win%36d\n", global.win);
	printf(" Lose%35d\n", global.lose);
	printf(" Push%35d\n", global.push);
	printf(" Bust%35d\n", global.bust);
	printf(" Withdraval%29d\n", global.quit);
	printf(" BlackJack%30d\n", global.blackjack);
	printf(" BlackJack push%25d\n", global.bjPush);
	printf(" Max cards drawn in one game%12d\n\n", global.maxCards);
	if (global.games != 0) printf(" Winrate%31.1f%%\n\n", (winrate * 100));
	  
	printf(" Press enter to proceed to the main menu...\n");
}

// Display and count umber of unlocked achievements;
void showAchievements(const stats global)
{
	int c = 0;

	if (global.games >= 1) {
		printf("\n\n [Common] Enter the casino: Play your first game\n");
		c++;
	}
	if (global.games >= 50) {
		printf(" [Rare] Another day, another card: Play 50 games\n");
		c++;
	}
	if (global.games >= 100) {
		printf(" [Epic] Gambler: Play 100 games\n");
		c++;
	}
	if (global.games >= 500) {
		printf(" [Legendary] Card-o-holic: Play 500 games\n");
		c++;
	}
	if (global.win >= 3) {
		printf(" [Common] Taste of victory: Win 5 games\n");
		c++;
	}
	if (global.win >= 25) {
		printf(" [Common] Double bet!: Win 25 games\n");
		c++;
	}
	if (global.win >= 50) {
		printf(" [Rare] Poker Face: Win 50 games\n");
		c++;
	}
	if (global.win >= 100) {
		printf(" [Epic] I'll take the bank: Win 100 games\n");
		c++;
	}
	if (global.win >= 500) {
		printf(" [Legendary] Legendary gambler: Win 500 games\n");
		c++;
	}
	if ((float)global.win / global.games > 0.5 && global.games >= 10) {
		printf(" [Rare] This is how cosino works: Win 0.5 of the games\n");
		c++;
	}
	if (global.blackjack >= 1) {
		printf(" [Rare] Blackjack!: Draw blackjack\n");
		c++;
	}
	if (global.blackjack >= 5) {
		printf(" [Epic] You think I'm bluffing?: Draw 5 blackjacks\n");
		c++;
	}
	if (global.blackjack >= 20) {
		printf(" [Legendary] Golden hand: Draw 20 blackjacks\n");
		c++;
	}
	if (global.bjPush >= 1) {
		printf(" [Legendary] 1 out of 2500: Blackjack push\n");
		c++;
	}
	if (global.bust >= 5) {
		printf(" [Common] Busted: You have busted 5 time\n");
		c++;
	}
	if (global.bust >= 20) {
		printf(" [Common] Better luck next time: You have busted 20 times\n");
		c++;
	}
	if (global.bust >= 50) {
		printf(" [Rare] *Boom*: You have busted 50 times\n");
		c++;
	}
	if (global.bust >= 100) {
		printf(" [Legendary] Ghostbuster: You have busted 100 times\n");
		c++;
	}
	if (global.lose >= 5) {
		printf(" [Common] We all have a bad day sometimes: Lose 5 games\n");
		c++;
	}
	if (global.lose >= 50) {
		printf(" [Rare] I'll recoup!: You have busted 50 times\n");
		c++;
	}
	if (global.lose >= 200) {
		printf(" [Legendary] Bankrupt: You  lost 200 times\n");
		c++;
	}
	if (global.maxCards >= 3) {
		printf(" [Common] Hit me: Draw 3 cards in one game\n");
		c++;
	}
	if (global.maxCards >= 4) {
		printf(" [Rare] Risk-lover: Draw 4 cards in one game\n");
		c++;
	}
	if (global.maxCards >= 5) {
		printf(" [Epic] Gimme mo': Draw 5 cards in one game\n");
		c++;
	}
	if (global.maxCards >= 6) {
		printf(" [Epic] Another one in the basket: Draw 6 cards in one game\n");
		c++;
	}
	if (global.maxCards >= 7) {
		printf(" [Legendary] All-in: Draw 7 cards in one game\n");
		c++;
	}
	if (global.quit >= 1) {
		printf(" [Rare] Ace?!: Withdraw your bet\n");
		c++;
	}
	if (global.quit >= 5) {
		printf(" [Epic] You gotta be kidding: Withdraw your bet 5 times\n");
		c++;
	}
	if (global.quit >= 15) {
		printf(" [Legendary] Risk-management: Withdraw your bet 15 times\n");
		c++;
	}
	if (global.push >= 1) {
		printf(" [Common] Ok, I guess: You got push one time\n");
		c++;
	}
	if (global.push >= 10) {
		printf(" [Rare] That's a match!: You got push 10 times\n");
		c++;
	}
	if (global.push >= 25) {
		printf(" [Legendary] We could be partners: You got push 25 times\n");
		c++;
	}
	
	if (c == 0) printf("\n You don't have any achievemetns yet!\n");
	else printf("\n There are %d more ahievements to unlock!\n", 32 - c);
	printf(" Press enter to proceed to the main menu...\n\n");
}