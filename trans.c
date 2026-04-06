// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure clientData

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void viewAccount(FILE *fPtr);       // NEW: Feature 1 - View specific account
void viewAllAccounts(FILE *fPtr);   // NEW: Feature 2 - View all accounts
void transferFunds(FILE *fPtr);     // NEW: Feature 3 - Transfer funds between accounts

// Utility functions for safe input
int safeInputInt(const char* prompt, unsigned int *out_val) {
    char buffer[256];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
    return sscanf(buffer, "%u", out_val) == 1;
}

int safeInputDouble(const char* prompt, double *out_val) {
    char buffer[256];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
    return sscanf(buffer, "%lf", out_val) == 1;
}

int safeInputString(const char* prompt, char* out_val, int max_len) {
    printf("%s", prompt);
    char fmt[32];
    sprintf(fmt, "%%%ds", max_len-1);
    if (scanf(fmt, out_val) != 1) {
        return 0;
    }
    // flush rest of line
    int c;
    while((c = getchar()) != '\n' && c != EOF);
    return 1;
}


int main(void)
{
    FILE *cfPtr;         // clients.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; exits if file cannot be opened
    // SWITCHED to clients.dat to prevent struct padding misalignment from old credit.dat 
    if ((cfPtr = fopen("clients.dat", "rb+")) == NULL)
    {
        printf("File 'clients.dat' could not be opened for update. Attempting to create it...\n");
        if ((cfPtr = fopen("clients.dat", "wb+")) == NULL) {
            printf("Failed to create file 'clients.dat'. Exiting.\n");
            exit(-1);
        }
        
        // Initialize 100 blank records
        struct clientData blankClient = {0, "", "", 0.0};
        for (int i = 0; i < 100; i++) {
            fwrite(&blankClient, sizeof(struct clientData), 1, cfPtr);
        }
        rewind(cfPtr);
        printf("Created new 'clients.dat' with 100 blank records.\n");
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 8)  
    {
        switch (choice)
        {
        // create text file from record file
        case 1:
            textFile(cfPtr);
            break;
        // update record
        case 2:
            updateRecord(cfPtr);
            break;
        // create record
        case 3:
            newRecord(cfPtr);
            break;
        // delete existing record
        case 4:
            deleteRecord(cfPtr);
            break;
        // NEW Feature 1: View specific account
        case 5:
            viewAccount(cfPtr);
            break;
        // NEW Feature 2: View ALL accounts
        case 6:
            viewAllAccounts(cfPtr);
            break;
        // NEW Feature 3: Transfer funds between accounts
        case 7:
            transferFunds(cfPtr);
            break;
        // display if user does not select valid choice
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
    return 0;
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    struct clientData client = {0, "", "", 0.0};

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } // end if
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // copy all records from random-access file into text file
        while (fread(&client, sizeof(struct clientData), 1, readPtr) == 1)
        {
            // write single record to text file
            if (client.acctNum != 0)
            {
                fprintf(writePtr, "%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName,
                        client.balance);
            } // end if
        }     // end while

        fclose(writePtr); // fclose closes the file
        puts("Formatted text file 'accounts.txt' created successfully.");
    }                     // end else
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    unsigned int account; // account number
    double transaction;   // transaction amount
    struct clientData client = {0, "", "", 0.0};

    if (!safeInputInt("Enter account to update ( 1 - 100 ): ", &account) || account < 1 || account > 100) {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
    }
    else
    { // update record
        printf("%-6d%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        if (!safeInputDouble("Enter charge ( - ) or payment ( + ): ", &transaction)) {
            puts("Invalid transaction amount.");
            return;
        }
        
        client.balance += transaction; // update record balance

        printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

        // move file pointer to correct record in file
        fseek(fPtr, (account - 1) * sizeof(struct clientData), SEEK_SET);
        // write updated record over old record in file
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
        puts("Account updated.");
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                       // stores record read from file
    struct clientData blankClient = {0, "", "", 0.0}; // blank client
    unsigned int accountNum;                        // account number

    if (!safeInputInt("Enter account number to delete ( 1 - 100 ): ", &accountNum) || accountNum < 1 || accountNum > 100) {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);
    
    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", accountNum);
    } 
    else
    { // delete record
        fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&blankClient, sizeof(struct clientData), 1, fPtr);
        printf("Account %u deleted successfully.\n", accountNum);
    } 
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum; 

    if (!safeInputInt("Enter new account number ( 1 - 100 ): ", &accountNum) || accountNum < 1 || accountNum > 100) {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record in file
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    // read record from file
    fread(&client, sizeof(struct clientData), 1, fPtr);
    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
    } 
    else
    { // create record
        if (!safeInputString("Enter lastname: ", client.lastName, 15)) return;
        if (!safeInputString("Enter firstname: ", client.firstName, 10)) return;

        char balanceStr[256];
        printf("Enter balance: ");
        if (!fgets(balanceStr, sizeof(balanceStr), stdin) || sscanf(balanceStr, "%lf", &client.balance) != 1) {
            puts("Invalid balance.");
            return;
        }

        client.acctNum = accountNum;
        // insert record in file
        fseek(fPtr, (client.acctNum - 1) * sizeof(struct clientData), SEEK_SET);
        fwrite(&client, sizeof(struct clientData), 1, fPtr);
        printf("Account %u created successfully.\n", accountNum);
    } 
} // end function newRecord

// NEW FEATURE 1: View specific account details
void viewAccount(FILE *fPtr)
{
    unsigned int accountNum; 
    struct clientData client = {0, "", "", 0.0};

    if (!safeInputInt("Enter account number to view ( 1 - 100 ): ", &accountNum) || accountNum < 1 || accountNum > 100) {
        puts("Invalid account number.");
        return;
    }

    // move file pointer to correct record
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&client, sizeof(struct clientData), 1, fPtr);

    // check if account exists
    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", accountNum);
    }
    else
    {
        printf("\n%-20s\n", "ACCOUNT DETAILS");
        printf("%-20s\n", "---------------");
        printf("%-15s: %u\n", "Account Number", client.acctNum);
        printf("%-15s: %s\n", "Last Name", client.lastName);
        printf("%-15s: %s\n", "First Name", client.firstName);
        printf("%-15s: $%.2f\n", "Balance", client.balance);
        printf("\n");
    }
} // end function viewAccount

// NEW FEATURE 2: View ALL accounts
void viewAllAccounts(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    int found = 0;
    
    rewind(fPtr);
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("---------------------------------------------\n");
    
    while(fread(&client, sizeof(struct clientData), 1, fPtr) == 1) {
        if(client.acctNum != 0) {
            printf("%-6d%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);
            found++;
        }
    }
    
    if (found == 0) {
        printf("No accounts found in the database.\n");
    } else {
        printf("---------------------------------------------\n");
        printf("Total Accounts: %d\n\n", found);
    }
}

// NEW FEATURE 3: Transfer funds between accounts
void transferFunds(FILE *fPtr)
{
    unsigned int sourceAcct;      
    unsigned int destAcct;        
    double amount;                
    struct clientData sourceClient = {0, "", "", 0.0};
    struct clientData destClient = {0, "", "", 0.0};

    if (!safeInputInt("Enter source account number ( 1 - 100 ): ", &sourceAcct) || sourceAcct < 1 || sourceAcct > 100) {
        puts("Invalid source account.");
        return;
    }

    fseek(fPtr, (sourceAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&sourceClient, sizeof(struct clientData), 1, fPtr);

    if (sourceClient.acctNum == 0)
    {
        printf("Source account #%u does not exist.\n", sourceAcct);
        return;
    }

    if (!safeInputInt("Enter destination account number ( 1 - 100 ): ", &destAcct) || destAcct < 1 || destAcct > 100) {
        puts("Invalid destination account.");
        return;
    }

    fseek(fPtr, (destAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fread(&destClient, sizeof(struct clientData), 1, fPtr);

    if (destClient.acctNum == 0)
    {
        printf("Destination account #%u does not exist.\n", destAcct);
        return;
    }

    if (sourceAcct == destAcct)
    {
        printf("Source and destination accounts cannot be the same.\n");
        return;
    }

    printf("Current balance in source account: $%.2f\n", sourceClient.balance);
    if (!safeInputDouble("Enter amount to transfer: $", &amount)) {
        puts("Invalid amount.");
        return;
    }

    if (amount <= 0)
    {
        printf("Transfer amount must be positive.\n");
        return;
    }

    if (amount > sourceClient.balance)
    {
        printf("Insufficient funds. Current balance: $%.2f\n", sourceClient.balance);
        return;
    }

    // perform the transfer
    sourceClient.balance -= amount;
    destClient.balance += amount;

    // write updated source record
    fseek(fPtr, (sourceAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&sourceClient, sizeof(struct clientData), 1, fPtr);

    // write updated destination record
    fseek(fPtr, (destAcct - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(&destClient, sizeof(struct clientData), 1, fPtr);

    printf("\nTransfer completed successfully!\n");
    printf("Source account #%u new balance: $%.2f\n", sourceAcct, sourceClient.balance);
    printf("Destination account #%u new balance: $%.2f\n", destAcct, destClient.balance);
} 

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; 
    
    printf("\nEnter your choice\n"
             "1 - store a formatted text file of accounts called\n"
             "    \"accounts.txt\" for printing\n"
             "2 - update an account\n"
             "3 - add a new account\n"
             "4 - delete an account\n"
             "5 - view an account\n"           
             "6 - view ALL accounts\n"          // NEW: Feature 2
             "7 - transfer funds\n"            
             "8 - end program\n? ");            

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        if (sscanf(buffer, "%u", &menuChoice) == 1) {
            return menuChoice;
        }
    }
    
    return 0; // return an invalid choice if input parsing fails
} 
