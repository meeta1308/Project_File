#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

void registerUser()
{
    string username, password, age;
    cout << "\n--- Register ---\n";
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;
    cout << "Enter age: ";
    cin >> age;

    ofstream file("file.txt", ios::app);
    if (file.is_open())
    {
        file << username << " " << password << " " << age << "\n";
        file.close();
        cout << "✅ Registration successful!\n";
    }
    else
    {
        cout << "❌ Error opening file.\n";
    }
}

void loginUser()
{
    string username, password;
    string line, storedUser, storedPass, storedAge;
    bool success = false;

    cout << "\n--- Login ---\n";
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    ifstream file("file.txt");
    if (!file.is_open())
    {
        cout << "❌ Error opening file.\n";
        return;
    }

    while (getline(file, line))
    {
        istringstream iss(line);
        iss >> storedUser >> storedPass >> storedAge;
        if (username == storedUser && password == storedPass)
        {
            success = true;
            break;
        }
    }
    file.close();

    if (success)
    {
        cout << "✅ Login successful!\n";
        cout << "Welcome, " << storedUser << " (Age: " << storedAge << ")\n";
    }
    else
    {
        cout << "❌ Login failed: Incorrect username or password.\n";
    }
}

void changePassword()
{
    string username, oldPass, newPass1, newPass2;
    string line, storedUser, storedPass, storedAge;
    string lines[1000]; // max 1000 users
    int count = 0;
    bool found = false;
    bool passwordChanged = false;

    cout << "\n--- Change Password ---\n";
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter old password: ";
    cin >> oldPass;

    ifstream file("file.txt");
    if (!file.is_open())
    {
        cout << "❌ Error opening file.\n";
        return;
    }

    while (getline(file, line))
    {
        istringstream iss(line);
        iss >> storedUser >> storedPass >> storedAge;

        if (storedUser == username && storedPass == oldPass)
        {
            found = true;
            cout << "Enter new password: ";
            cin >> newPass1;
            cout << "Confirm new password: ";
            cin >> newPass2;

            if (newPass1 == newPass2)
            {
                lines[count++] = storedUser + " " + newPass1 + " " + storedAge;
                passwordChanged = true;
            }
            else
            {
                lines[count++] = storedUser + " " + storedPass + " " + storedAge;
                cout << "❌ New passwords do not match. Password not changed.\n";
            }
        }
        else
        {
            lines[count++] = storedUser + " " + storedPass + " " + storedAge;
        }
    }
    file.close();

    if (!found)
    {
        cout << "❌ Password change failed: Incorrect username or old password.\n";
        return;
    }

    ofstream outFile("file.txt");
    for (int i = 0; i < count; ++i)
    {
        outFile << lines[i] << "\n";
    }
    outFile.close();

    if (passwordChanged)
    {
        cout << "✅ Password changed successfully!\n";
    }
}

int main()
{
    int choice;
    do
    {
        cout << "\n🔐 Security System Menu 🔐\n";
        cout << "1. Register\n";
        cout << "2. Login\n";
        cout << "3. Change Password\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            registerUser();
            break;
        case 2:
            loginUser();
            break;
        case 3:
            changePassword();
            break;
        case 4:
            cout << "👋 Exiting... Thank you!\n";
            break;
        default:
            cout << "❗ Invalid choice. Try again.\n";
        }
    } while (choice != 4);

    return 0;
}