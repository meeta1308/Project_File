#include <iostream>
#include <string>
using namespace std;

// Caesar Cipher Encryption
string encrypt(string text, int key = 3)
{
    string result = "";
    for (char c : text)
    {
        if (isalpha(c))
        {
            char base = islower(c) ? 'a' : 'A';
            result += char((c - base + key) % 26 + base);
        }
        else
        {
            result += c;
        }
    }
    return result;
}

// Caesar Cipher Decryption
string decrypt(string text, int key = 3)
{
    return encrypt(text, 26 - key); // reverse shift
}

void chatSession(string user1, string user2)
{
    string message;
    int choice;

    do
    {
        cout << "\n1. " << user1 << " sends message\n";
        cout << "2. " << user2 << " sends message\n";
        cout << "3. Exit chat\n";
        cout << "Enter choice: ";
        cin >> choice;
        cin.ignore(); // clear input buffer

        switch (choice)
        {
        case 1:
            cout << user1 << " > ";
            getline(cin, message);
            cout << user2 << " received: " << decrypt(encrypt(message)) << "\n";
            break;
        case 2:
            cout << user2 << " > ";
            getline(cin, message);
            cout << user1 << " received: " << decrypt(encrypt(message)) << "\n";
            break;
        case 3:
            cout << "👋 Chat ended.\n";
            break;
        default:
            cout << "❗ Invalid choice.\n";
        }
    } while (choice != 3);
}

int main()
{
    string user1, user2;
    cout << "Enter name of User 1: ";
    cin >> user1;
    cout << "Enter name of User 2: ";
    cin >> user2;

    chatSession(user1, user2);
    return 0;
}