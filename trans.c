#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ACCOUNTS 100
#define PRIMARY_DATA_FILE "credit.dat"
#define ALT_DATA_FILE "clients.dat"
#define TEXT_FILE "accounts.txt"

typedef struct
{
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
} ClientData;

unsigned int enterChoice(void);
int isValidAccount(unsigned int account);
int loadRecord(FILE *fPtr, unsigned int account, ClientData *client);
int saveRecord(FILE *fPtr, const ClientData *client);
void clearInputBuffer(void);
unsigned int readAccountNumber(const char *message);
double readAmount(const char *message);
void printHeader(void);
void printRecord(const ClientData *client);
FILE *openBankFile(char *activeFileName, size_t size);
int isUsableDataFile(const char *fileName);
void createEmptyDataFile(const char *fileName);

void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void viewRecord(FILE *fPtr);
void listRecords(FILE *fPtr);
void depositMoney(FILE *fPtr);
void withdrawMoney(FILE *fPtr);
void transferMoney(FILE *fPtr);
void searchByLastName(FILE *fPtr);
void showSummary(FILE *fPtr);

int main(void)
{
    FILE *cfPtr;
    unsigned int choice;
    char activeFileName[64] = "";

    cfPtr = openBankFile(activeFileName, sizeof(activeFileName));
    if (cfPtr == NULL)
    {
        puts("Bank data file could not be opened.");
        return 1;
    }

    printf("Using data file: %s\n", activeFileName);

    while ((choice = enterChoice()) != 12)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;
        case 2:
            updateRecord(cfPtr);
            break;
        case 3:
            newRecord(cfPtr);
            break;
        case 4:
            deleteRecord(cfPtr);
            break;
        case 5:
            viewRecord(cfPtr);
            break;
        case 6:
            listRecords(cfPtr);
            break;
        case 7:
            depositMoney(cfPtr);
            break;
        case 8:
            withdrawMoney(cfPtr);
            break;
        case 9:
            transferMoney(cfPtr);
            break;
        case 10:
            searchByLastName(cfPtr);
            break;
        case 11:
            showSummary(cfPtr);
            break;
        default:
            puts("Incorrect choice. Please enter 1 to 12.");
            break;
        }
    }

    fclose(cfPtr);
    puts("Program ended.");
    return 0;
}

FILE *openBankFile(char *activeFileName, size_t size)
{
    FILE *fPtr;

    if (isUsableDataFile(PRIMARY_DATA_FILE))
    {
        strncpy(activeFileName, PRIMARY_DATA_FILE, size - 1);
        activeFileName[size - 1] = '\0';
        return fopen(PRIMARY_DATA_FILE, "rb+");
    }

    if (isUsableDataFile(ALT_DATA_FILE))
    {
        strncpy(activeFileName, ALT_DATA_FILE, size - 1);
        activeFileName[size - 1] = '\0';
        return fopen(ALT_DATA_FILE, "rb+");
    }

    createEmptyDataFile(PRIMARY_DATA_FILE);
    fPtr = fopen(PRIMARY_DATA_FILE, "rb+");
    if (fPtr != NULL)
    {
        strncpy(activeFileName, PRIMARY_DATA_FILE, size - 1);
        activeFileName[size - 1] = '\0';
    }

    return fPtr;
}

int isUsableDataFile(const char *fileName)
{
    FILE *fPtr;
    long fileSize;

    fPtr = fopen(fileName, "rb");
    if (fPtr == NULL)
    {
        return 0;
    }

    if (fseek(fPtr, 0, SEEK_END) != 0)
    {
        fclose(fPtr);
        return 0;
    }

    fileSize = ftell(fPtr);
    fclose(fPtr);

    if (fileSize < (long)(MAX_ACCOUNTS * sizeof(ClientData)))
    {
        return 0;
    }

    if (fileSize % (long)sizeof(ClientData) != 0)
    {
        return 0;
    }

    return 1;
}

void createEmptyDataFile(const char *fileName)
{
    FILE *fPtr;
    ClientData blank = {0, "", "", 0.0};
    int i;

    fPtr = fopen(fileName, "wb");
    if (fPtr == NULL)
    {
        return;
    }

    for (i = 0; i < MAX_ACCOUNTS; i++)
    {
        fwrite(&blank, sizeof(ClientData), 1, fPtr);
    }

    fclose(fPtr);
}

unsigned int enterChoice(void)
{
    unsigned int menuChoice;

    printf("\nEnter your choice\n"
           "1  - export accounts to accounts.txt\n"
           "2  - update an account balance (+/-)\n"
           "3  - add a new account\n"
           "4  - delete an account\n"
           "5  - view one account\n"
           "6  - list all accounts\n"
           "7  - deposit money\n"
           "8  - withdraw money\n"
           "9  - transfer money\n"
           "10 - search by last name\n"
           "11 - show bank summary\n"
           "12 - end program\n? ");

    if (scanf("%u", &menuChoice) != 1)
    {
        clearInputBuffer();
        return 0;
    }

    return menuChoice;
}

void clearInputBuffer(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }
}

int isValidAccount(unsigned int account)
{
    return account >= 1 && account <= MAX_ACCOUNTS;
}

unsigned int readAccountNumber(const char *message)
{
    unsigned int account;

    printf("%s", message);
    if (scanf("%u", &account) != 1)
    {
        clearInputBuffer();
        return 0;
    }

    if (!isValidAccount(account))
    {
        return 0;
    }

    return account;
}

double readAmount(const char *message)
{
    double amount;

    printf("%s", message);
    if (scanf("%lf", &amount) != 1)
    {
        clearInputBuffer();
        return -1.0;
    }

    return amount;
}

int loadRecord(FILE *fPtr, unsigned int account, ClientData *client)
{
    if (!isValidAccount(account))
    {
        return 0;
    }

    if (fseek(fPtr, (long)(account - 1) * (long)sizeof(ClientData), SEEK_SET) != 0)
    {
        return 0;
    }

    return fread(client, sizeof(ClientData), 1, fPtr) == 1;
}

int saveRecord(FILE *fPtr, const ClientData *client)
{
    if (!isValidAccount(client->acctNum))
    {
        return 0;
    }

    if (fseek(fPtr, (long)(client->acctNum - 1) * (long)sizeof(ClientData), SEEK_SET) != 0)
    {
        return 0;
    }

    if (fwrite(client, sizeof(ClientData), 1, fPtr) != 1)
    {
        return 0;
    }

    fflush(fPtr);
    return 1;
}

void printHeader(void)
{
    printf("%-6s %-14s %-10s %12s\n", "Acct", "Last Name", "First Name", "Balance");
}

void printRecord(const ClientData *client)
{
    printf("%-6u %-14s %-10s %12.2f\n",
           client->acctNum,
           client->lastName,
           client->firstName,
           client->balance);
}

void textFile(FILE *readPtr)
{
    FILE *writePtr;
    ClientData client;
    unsigned int i;
    int found = 0;

    writePtr = fopen(TEXT_FILE, "w");
    if (writePtr == NULL)
    {
        puts("File could not be opened.");
        return;
    }

    fprintf(writePtr, "%-6s %-14s %-10s %12s\n", "Acct", "Last Name", "First Name", "Balance");

    for (i = 1; i <= MAX_ACCOUNTS; i++)
    {
        if (loadRecord(readPtr, i, &client) && client.acctNum != 0)
        {
            fprintf(writePtr, "%-6u %-14s %-10s %12.2f\n",
                    client.acctNum,
                    client.lastName,
                    client.firstName,
                    client.balance);
            found = 1;
        }
    }

    fclose(writePtr);

    if (found)
    {
        printf("Accounts exported to %s successfully.\n", TEXT_FILE);
    }
    else
    {
        puts("No active accounts to export.");
    }
}

void updateRecord(FILE *fPtr)
{
    unsigned int accountNum = readAccountNumber("Enter account number to update (1 - 100): ");
    ClientData client = {0, "", "", 0.0};
    double transaction;

    if (!loadRecord(fPtr, accountNum, &client) || client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", accountNum);
        return;
    }

    printHeader();
    printRecord(&client);

    transaction = readAmount("Enter charge (+) or payment (-): ");
    if (transaction == -1.0)
    {
        puts("Invalid amount.");
        return;
    }

    client.balance += transaction;
    if (!saveRecord(fPtr, &client))
    {
        puts("Account could not be updated.");
        return;
    }

    puts("Account updated successfully.");
    printHeader();
    printRecord(&client);
}

void newRecord(FILE *fPtr)
{
    unsigned int accountNum = readAccountNumber("Enter new account number (1 - 100): ");
    ClientData client = {0, "", "", 0.0};

    if (!loadRecord(fPtr, accountNum, &client))
    {
        puts("Invalid account number.");
        return;
    }

    if (client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
        return;
    }

    printf("Enter last name, first name, balance\n? ");
    if (scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance) != 3)
    {
        clearInputBuffer();
        puts("Invalid input.");
        return;
    }

    client.acctNum = accountNum;
    if (!saveRecord(fPtr, &client))
    {
        puts("New account could not be added.");
        return;
    }

    puts("New account added successfully.");
}

void deleteRecord(FILE *fPtr)
{
    unsigned int accountNum = readAccountNumber("Enter account number to delete (1 - 100): ");
    ClientData client = {0, "", "", 0.0};
    ClientData blankClient = {0, "", "", 0.0};

    if (!loadRecord(fPtr, accountNum, &client) || client.acctNum == 0)
    {
        printf("Account #%u does not exist.\n", accountNum);
        return;
    }

    if (fseek(fPtr, (long)(accountNum - 1) * (long)sizeof(ClientData), SEEK_SET) != 0)
    {
        puts("Account could not be deleted.");
        return;
    }

    if (fwrite(&blankClient, sizeof(ClientData), 1, fPtr) != 1)
    {
        puts("Account could not be deleted.");
        return;
    }

    fflush(fPtr);
    puts("Account deleted successfully.");
}

void viewRecord(FILE *fPtr)
{
    unsigned int account = readAccountNumber("Enter account number to view (1 - 100): ");
    ClientData client = {0, "", "", 0.0};

    if (!loadRecord(fPtr, account, &client) || client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    printHeader();
    printRecord(&client);
}

void listRecords(FILE *fPtr)
{
    ClientData client;
    unsigned int i;
    int found = 0;

    printHeader();
    for (i = 1; i <= MAX_ACCOUNTS; i++)
    {
        if (loadRecord(fPtr, i, &client) && client.acctNum != 0)
        {
            printRecord(&client);
            found = 1;
        }
    }

    if (!found)
    {
        puts("No active accounts found.");
    }
}

void depositMoney(FILE *fPtr)
{
    unsigned int account = readAccountNumber("Enter account number to deposit into (1 - 100): ");
    double amount;
    ClientData client = {0, "", "", 0.0};

    if (!loadRecord(fPtr, account, &client) || client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    amount = readAmount("Enter deposit amount: ");
    if (amount <= 0)
    {
        puts("Invalid deposit amount.");
        return;
    }

    client.balance += amount;
    if (!saveRecord(fPtr, &client))
    {
        puts("Deposit failed.");
        return;
    }

    puts("Deposit successful.");
    printHeader();
    printRecord(&client);
}

void withdrawMoney(FILE *fPtr)
{
    unsigned int account = readAccountNumber("Enter account number to withdraw from (1 - 100): ");
    double amount;
    ClientData client = {0, "", "", 0.0};

    if (!loadRecord(fPtr, account, &client) || client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
        return;
    }

    amount = readAmount("Enter withdrawal amount: ");
    if (amount <= 0)
    {
        puts("Invalid withdrawal amount.");
        return;
    }

    if (amount > client.balance)
    {
        puts("Insufficient balance.");
        return;
    }

    client.balance -= amount;
    if (!saveRecord(fPtr, &client))
    {
        puts("Withdrawal failed.");
        return;
    }

    puts("Withdrawal successful.");
    printHeader();
    printRecord(&client);
}

void transferMoney(FILE *fPtr)
{
    unsigned int fromAccount = readAccountNumber("Enter sender account number (1 - 100): ");
    unsigned int toAccount = readAccountNumber("Enter receiver account number (1 - 100): ");
    double amount;
    ClientData fromClient = {0, "", "", 0.0};
    ClientData toClient = {0, "", "", 0.0};

    if (fromAccount == 0 || toAccount == 0 || fromAccount == toAccount)
    {
        puts("Invalid account numbers for transfer.");
        return;
    }

    if (!loadRecord(fPtr, fromAccount, &fromClient) || fromClient.acctNum == 0)
    {
        printf("Sender account #%u not found.\n", fromAccount);
        return;
    }

    if (!loadRecord(fPtr, toAccount, &toClient) || toClient.acctNum == 0)
    {
        printf("Receiver account #%u not found.\n", toAccount);
        return;
    }

    amount = readAmount("Enter transfer amount: ");
    if (amount <= 0)
    {
        puts("Invalid transfer amount.");
        return;
    }

    if (amount > fromClient.balance)
    {
        puts("Insufficient balance in sender account.");
        return;
    }

    fromClient.balance -= amount;
    toClient.balance += amount;

    if (!saveRecord(fPtr, &fromClient) || !saveRecord(fPtr, &toClient))
    {
        puts("Transfer failed.");
        return;
    }

    puts("Transfer successful.");
    puts("Sender account:");
    printHeader();
    printRecord(&fromClient);
    puts("Receiver account:");
    printHeader();
    printRecord(&toClient);
}

void searchByLastName(FILE *fPtr)
{
    char search[15];
    ClientData client;
    unsigned int i;
    int found = 0;

    printf("Enter last name to search: ");
    if (scanf("%14s", search) != 1)
    {
        clearInputBuffer();
        puts("Invalid name input.");
        return;
    }

    printHeader();
    for (i = 1; i <= MAX_ACCOUNTS; i++)
    {
        if (loadRecord(fPtr, i, &client) && client.acctNum != 0 && strcmp(client.lastName, search) == 0)
        {
            printRecord(&client);
            found = 1;
        }
    }

    if (!found)
    {
        printf("No accounts found for last name: %s\n", search);
    }
}

void showSummary(FILE *fPtr)
{
    ClientData client;
    ClientData maxClient = {0, "", "", 0.0};
    ClientData minClient = {0, "", "", 0.0};
    unsigned int i;
    int count = 0;
    double total = 0.0;
    double average;

    for (i = 1; i <= MAX_ACCOUNTS; i++)
    {
        if (loadRecord(fPtr, i, &client) && client.acctNum != 0)
        {
            total += client.balance;
            if (count == 0 || client.balance > maxClient.balance)
            {
                maxClient = client;
            }
            if (count == 0 || client.balance < minClient.balance)
            {
                minClient = client;
            }
            count++;
        }
    }

    if (count == 0)
    {
        puts("No active accounts available for summary.");
        return;
    }

    average = total / count;

    printf("\nBank Summary\n");
    printf("Active accounts : %d\n", count);
    printf("Total balance   : %.2f\n", total);
    printf("Average balance : %.2f\n", average);
    printf("Highest balance : %u - %s %s - %.2f\n",
           maxClient.acctNum,
           maxClient.firstName,
           maxClient.lastName,
           maxClient.balance);
    printf("Lowest balance  : %u - %s %s - %.2f\n",
           minClient.acctNum,
           minClient.firstName,
           minClient.lastName,
           minClient.balance);
}
