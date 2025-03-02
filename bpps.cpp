#include <iostream>
#include <windows.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <conio.h>
#include <chrono>
#include <thread>
#include <fstream>
#include <string>
#include <iomanip>

using namespace std;

const int SCREEN_WIDTH = 120;
const int SCREEN_HEIGHT = 50;
const int SNOWFLAKE_COUNT = 80;

struct Snowflake
{
    int x, y;
};
struct userInfo
{
    string name;
    int score;
    int time;
};
const int MAX_USERS = 100000;
userInfo players[MAX_USERS];
int users;


string userName;
bool bullet = false, gameOver = false;

double turnTime = 2000;
double elapsed;
clock_t endTime, startTime;

int gameArr[1000][1000], loc = 16, turn = 0, point = 0, lives = 3;
int redDirect = 0;

int spaceY = 50, spaceX = 24;

int direction = 1, lastDirection = 0;

void hideCursor()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void setCursorPosition(int x, int y)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(hConsole, pos);
}

void start();
void mainMenu();

void useful()
{
    ifstream userInfoFile("winningData.txt");

    userInfoFile >> users;

    if (userInfoFile.is_open())
    {

        for (int i = 0; i < users; i++)
        {
            userInfoFile >> players[i].name;
            userInfoFile >> players[i].score;
            userInfoFile >> players[i].time;
        }
    }
    else
    {
        cout << "Unable to open winning.txt";
    }
    userInfoFile.close();
}
void leaderBoard()
{
    system("cls");
    useful();

    for (int i = 0; i < users; i++)
    {
        for (int j = i; j >= 1; j--)
        {
            if (players[j].score > players[j - 1].score)
                swap(players[j], players[j - 1]);
        }
    }
    cout << "-----------------------------------------------\n";
    cout << "| Rank | Name           | Score   | Time (s)  |\n";
    cout << "-----------------------------------------------\n";

    for (int i = 0; i < users; i++) {
        cout << "| " << setw(4) << i + 1 << " | "
             << setw(14) << players[i].name << " | "
             << setw(7) << players[i].score << " | "
             << setw(9) << players[i].time << " |\n";
    }
    cout << "-----------------------------------------------\n";

    getch();
}

void resetGameState()
{
    gameOver = false;
    bullet = false;
    point = 0;
    turn = 0;
    lives = 3;
    bullet = false;
}

// به‌روزرسانی موقعیت دانه‌های برف
void updateSnowflakes(Snowflake snowflakes[])
{
    for (int i = 0; i < SNOWFLAKE_COUNT; i++)
    {
        snowflakes[i].y++;
        if (snowflakes[i].y >= SCREEN_HEIGHT)
        {
            snowflakes[i].x = rand() % SCREEN_WIDTH;
            snowflakes[i].y = 0;
        }
    }
}

// رسم دانه‌های برف
void drawSnowflakes(const Snowflake snowflakes[])
{
    for (int i = 0; i < SNOWFLAKE_COUNT; i++)
    {
        setCursorPosition(snowflakes[i].x, snowflakes[i].y);
        cout << "\033[38;5;255m❆\033[0m";
    }
}

// پاک کردن دانه‌های برف قبلی
void clearSnowflakes(const Snowflake snowflakes[])
{
    for (int i = 0; i < SNOWFLAKE_COUNT; i++)
    {
        setCursorPosition(snowflakes[i].x, snowflakes[i].y);
        cout << " ";
    }
}

int dealWithNegatives(int negNum, int mod)
{
    negNum %= mod;
    if (negNum < 0)
        negNum += mod;

    return negNum;
}

void playLogoSound()
{
    // بستن فایل صوتی قبلی در صورت باز بودن
    mciSendString(TEXT("stop logoSound"), NULL, 0, NULL);
    mciSendString(TEXT("close logoSound"), NULL, 0, NULL);

    // باز کردن و پخش فایل صوتی برای نمایش لوگو
    mciSendString(TEXT("open \"epic-hybrid-logo-157092.mp3\" type mpegvideo alias logoSound"), NULL, 0, NULL);
    mciSendString(TEXT("play logoSound"), NULL, 0, NULL);
}

void playSelectionSound()
{
    // دستور برای پخش فایل MP3
    mciSendString(TEXT("close mySound"), NULL, 0, NULL); // بستن فایل قبلی
    mciSendString(TEXT("open \"mixkit-handgun-click-1660.mp3\" type mpegvideo alias mySound"), NULL, 0, NULL);
    mciSendString(TEXT("play mySound"), NULL, 0, NULL);
}

void playLoseSound()
{
    // بستن فایل صوتی قبلی در صورت باز بودن
    mciSendString(TEXT("stop loseSound"), NULL, 0, NULL);
    mciSendString(TEXT("close loseSound"), NULL, 0, NULL);

    // باز کردن و پخش فایل صوتی برای نمایش لوگو
    mciSendString(TEXT("open \"mixkit-cartoon-whistle-game-over-606.wav\" type mpegvideo alias loseSound"), NULL, 0, NULL);
    mciSendString(TEXT("play loseSound repeat"), NULL, 0, NULL);
}

void playBackgroundSound()
{
    // اگر قبلاً صدا باز شده، ببندش
    mciSendString(TEXT("stop bgSound"), NULL, 0, NULL);
    mciSendString(TEXT("close bgSound"), NULL, 0, NULL);

    // باز کردن و پخش فایل به صورت تکرار شونده (loop)
    mciSendString(TEXT("open \"squid_game_2_mingle.mp3\" type mpegvideo alias bgSound"), NULL, 0, NULL);
    mciSendString(TEXT("play bgSound repeat"), NULL, 0, NULL);
}

void stopBackgroundSound()
{
    mciSendString(TEXT("stop bgSound"), NULL, 0, NULL);
    mciSendString(TEXT("close bgSound"), NULL, 0, NULL);
}

void firstAppearLogo()
{
    hideCursor();
    setCursorPosition(0, 0);
    playLogoSound();

    for (int i = 1; i <= 200; i++)
    {
        if (_kbhit())
        {
            mciSendString(TEXT("stop logoSound"), NULL, 0, NULL);
            mciSendString(TEXT("close logoSound"), NULL, 0, NULL);
            break;
        }

        system("cls");
        cout << "\n\n\n";

        cout << "              \033[38;5;8m▒░░░░░▒▓░░░░░▒▓░░░▒▒░█░░░▒░░█░░▒▒░░█░░▒░░░▓░░▒░░░▓▒░░░░░▒▒░░░░░▒▒░░░░░▒▓░░░░░▒▓░░░░░░█░░░░░░▓░░▒░░░█\n";
        cout << "              ░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▒▒▒░▒░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▒▒▒\n";
        cout << "              ▒▒█▒██▓▒█▒▓▓▓▒█▒▒▓█▓█▓▒▒▓█▓▓▓▒▒█▒██▓░▓▒▒▓▓▒▓▒▒▒▓▓▓▓▒▒▒▓▒▓▓▒▒▓░▓▓▓░▓▒▒▒▓▒▓▒░░▒▒▒█▓▒▓█▒██▓▒█▒▓▓▓▒█▒░▒▓\n";
        cout << "              ▓░░░░▓░░░░░▓░░░░▒▓░░░░▓▓░░░░▓▒░░░░▓░░░░▒▓░░░░▒▓░░░░▓▒░░░░█▒░░░░▓░░░░▒▓░░░░▓▓░░░░█▒░░░░█▒░░░░▓░░░░▓▓░\n";
        cout << "              ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n";
        cout << "              ██████████████████████████████████████████████████████████████████████████\033[38;5;53m▒▒\033[38;5;8m███████\033[38;5;53m▒\033[38;5;8m████████████████\n";
        cout << "              ██████████████▓\033[38;5;53m░░░░░░░░░░░░░░░░░░\033[38;5;8m████\033[38;5;53m▒░░\033[38;5;8m█████\033[38;5;53m▒▒▒▒▒▒▒▒\033[38;5;8m██\033[38;5;53m▒▒▒▒▒▒▒▒▒\033[38;5;8m████████████\033[38;5;53m░\033[38;5;8m████▓\033[38;5;53m▒\033[38;5;8m█████████████████\n";
        cout << "              █████████████▓\033[38;5;53m░░\033[38;5;8m█████████\033[38;5;53m░░\033[38;5;8m█████\033[38;5;53m░░\033[38;5;8m▓█\033[38;5;53m▒░░░░\033[38;5;8m███\033[38;5;53m▒▒\033[38;5;8m████████████████\033[38;5;53m▒▒\033[38;5;8m█████████▓\033[38;5;53m░▒▒░░▒▒▒\033[38;5;8m▓\033[38;5;53m▒\033[38;5;8m▓▓██████████████\n";
        cout << "              ██████████████\033[38;5;53m░░░░░░░░\033[38;5;8m▓██\033[38;5;53m░░░░░░░░░\033[38;5;8m█\033[38;5;53m▒░▒\033[38;5;8m█\033[38;5;53m▒░▒\033[38;5;8m██\033[38;5;53m░░\033[38;5;8m███████████\033[38;5;53m▒▒▒▒▒▒▒\033[38;5;8m███████\033[38;5;53m▒░░░░░░░▒▒▒▒▒▒▒▒\033[38;5;8m█████████████\n";
        cout << "              █████████████████████\033[38;5;53m░░\033[38;5;8m██\033[38;5;53m░░\033[38;5;8m███████\033[38;5;53m▒░░░░░░░▒\033[38;5;8m█\033[38;5;53m░░\033[38;5;8m████████████████\033[38;5;53m▒▒\033[38;5;8m███████\033[38;5;53m▒░\033[38;5;8m█\033[38;5;53m░░░▒▒░▒▒▒▒\033[38;5;8m██\033[38;5;53m▒\033[38;5;8m█████████████\n";
        cout << "              ██████████████\033[38;5;53m░░░░░░░░▒\033[38;5;8m██\033[38;5;53m░░\033[38;5;8m███████\033[38;5;53m░░\033[38;5;8m█████\033[38;5;53m░░\033[38;5;8m█▓\033[38;5;53m░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m███████\033[38;5;53m▒▒\033[38;5;8m█\033[38;5;53m▒▒\033[38;5;8m███████\033[38;5;53m▒\033[38;5;8m██\033[38;5;53m▒\033[38;5;8m█████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ██████████████\033[38;5;53m░░\033[38;5;8m█\033[38;5;53m▒░▒\033[38;5;8m█████\033[38;5;53m░░\033[38;5;8m█▓\033[38;5;53m░▒\033[38;5;8m████████\033[38;5;53m░▒\033[38;5;8m█████\033[38;5;53m▒▒▒▒▒▒\033[38;5;8m▓████\033[38;5;53m▒▒▒▒▒▒▒▒▒▒▒\033[38;5;53m▒▒▒▒▒▒▒\033[38;5;8m█████\033[38;5;53m▒▒▒▒▒▒▒▒\033[38;5;8m████████████\n";
        cout << "              ██████████████\033[38;5;53m░░\033[38;5;8m█\033[38;5;53m░░░░\033[38;5;8m████\033[38;5;53m░░\033[38;5;8m█▓\033[38;5;53m░░\033[38;5;8m███\033[38;5;53m░░\033[38;5;8m██\033[38;5;53m░░░░\033[38;5;8m████\033[38;5;53m░▒\033[38;5;8m▓▓▓▓\033[38;5;53m▒▒▒\033[38;5;8m██\033[38;5;53m▒▒\033[38;5;8m▓▓█▓██▓█▓\033[38;5;53m▒▒\033[38;5;8m████\033[38;5;53m▒▒▒\033[38;5;8m█\033[38;5;53m▒▒▒\033[38;5;8m███████████████████\n";
        cout << "              ██████████████\033[38;5;53m░░\033[38;5;8m█\033[38;5;53m░░\033[38;5;8m▓\033[38;5;53m░░░\033[38;5;8m██\033[38;5;53m░░\033[38;5;8m█▓\033[38;5;53m░▒\033[38;5;8m█\033[38;5;53m▒░░\033[38;5;8m██\033[38;5;53m░░\033[38;5;8m██\033[38;5;53m░░\033[38;5;8m███\033[38;5;53m░░\033[38;5;8m██████\033[38;5;53m░▒\033[38;5;8m█\033[38;5;53m▒▒▒▒▒▒\033[38;5;8m▓███▓\033[38;5;53m▒▒▒▒▒▒\033[38;5;53m▒▒\033[38;5;8m██\033[38;5;53m▒▒▒▒▒▒▒▒\033[38;5;8m██████████████\n";
        cout << "              ██████████████\033[38;5;53m░░\033[38;5;8m█\033[38;5;53m░░\033[38;5;8m▓██\033[38;5;53m░░░░░\033[38;5;8m█▓\033[38;5;53m░░░░\033[38;5;8m▓██\033[38;5;53m░░░░░░░░\033[38;5;8m██\033[38;5;53m░░\033[38;5;8m█████\033[38;5;53m▒▒\033[38;5;8m▓█\033[38;5;53m▒▒\033[38;5;8m████████▓\033[38;5;53m▒\033[38;5;8m▓█▓\033[38;5;53m▒▒▒\033[38;5;8m███████████\033[38;5;53m▒▒\033[38;5;8m████████████\n";
        cout << "              ██████████████\033[38;5;53m░░\033[38;5;8m█\033[38;5;53m░░\033[38;5;8m▓████\033[38;5;53m░░░\033[38;5;8m█▓\033[38;5;53m░░░\033[38;5;8m███\033[38;5;53m░░▒\033[38;5;8m█████\033[38;5;53m░░\033[38;5;8m█\033[38;5;53m░░░░░░░░\033[38;5;8m▓██\033[38;5;53m▒▒▒▒▒▒▒▒▒\033[38;5;8m█▓\033[38;5;53m▒\033[38;5;8m█████\033[38;5;53m▒\033[38;5;8m█\033[38;5;53m▒▒▒▒▒▒▒▒▒\033[38;5;53m▒▒\033[38;5;8m▓████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ███████▓\033[38;5;70m▒▒▒▒\033[38;5;8m▓▓████████████▓▓▓█████▓▓▓███████████████▓\033[38;5;70m▒▒▒▒▒▒\033[38;5;8m█████████████████████\033[38;5;52m▒▒▒▒▒▒▒▒▒\033[38;5;8m███████████\n";
        cout << "              █████\033[38;5;70m▒▒▒▒▒▒▒▒▒▒\033[38;5;8m███████▓\033[38;5;70m▒\033[38;5;8m███\033[38;5;70m▒▒\033[38;5;8m█████\033[38;5;70m▒▒\033[38;5;8m▓███\033[38;5;70m▒\033[38;5;8m███████▓\033[38;5;70m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m████████████\033[38;5;52m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m███████\n";
        cout << "              ███▓\033[38;5;70m▒▒\033[38;5;8m▓██\033[38;5;70m▒▒▒\033[38;5;8m██\033[38;5;70m▒▒▒\033[38;5;8m█████▓\033[38;5;70m▒▒▒▒\033[38;5;8m██\033[38;5;70m▒▒▒▒▒\033[38;5;8m█▓\033[38;5;70m▒▒▒▒\033[38;5;8m██████\033[38;5;70m▒▒▒▒▒\033[38;5;8m███\033[38;5;70m▒▒▒\033[38;5;8m▓███\033[38;5;70m▒▒▒▒▒\033[38;5;8m█████████\033[38;5;52m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m▓█████\n";
        cout << "              ███▓\033[38;5;70m▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m█████▓\033[38;5;70m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m██████\033[38;5;70m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m███████\033[38;5;52m▒▒▒\033[38;5;8m▓▓\033[38;5;52m▒▒▒\033[38;5;8m▓▓\033[38;5;52m▒▒▒\033[38;5;8m▓▓\033[38;5;52m▒▒▒\033[38;5;8m▓▓\033[38;5;52m▒▒▒\033[38;5;8m████\n";
        cout << "              ██████▓\033[38;5;70m▒\033[38;5;8m▓███\033[38;5;70m▒▒\033[38;5;8m██████████\033[38;5;70m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m████████████\033[38;5;70m▒▒▒▒▒▒▒▒▒▒\033[38;5;8m██████████████\033[38;5;52m▒▒▒▒▒\033[38;5;8m███\033[38;5;52m▒▒▒\033[38;5;8m████\033[38;5;52m▒▒▒▒\033[38;5;8m▓█████\n";
        cout << "              █████\033[38;5;70m▒\033[38;5;8m▓██\033[38;5;70m▒▒▒\033[38;5;8m██\033[38;5;70m▒\033[38;5;8m███████████\033[38;5;70m▒\033[38;5;8m████████▓\033[38;5;70m▒\033[38;5;8m████████████\033[38;5;70m▒▒▒\033[38;5;8m▓█\033[38;5;70m▒▒▒\033[38;5;8m▓█\033[38;5;70m▒▒▒\033[38;5;8m▓█████████████▓\033[38;5;52m▒▒\033[38;5;8m█████████████\033[38;5;52m▒▒\033[38;5;8m██████\n";
        cout << "              ███\033[38;5;70m▒▒\033[38;5;8m██\033[38;5;70m▒▒\033[38;5;8m███\033[38;5;70m▒▒\033[38;5;8m█\033[38;5;70m▒▒\033[38;5;8m███████\033[38;5;70m▒▒\033[38;5;8m███████████\033[38;5;70m▒▒\033[38;5;8m▓████████████████████████████████████████████████████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒\n";
        cout << "              ▓░░░▓▒░░░▓░░░▒█░░░▒▓░░░▓▓░░░▓▓░░░▓▓░░░▓▒░░░▓▒░░░█░░░▒▓░░░▒▒░░░▓▓░░░░░░░░▓▓░░░▒▒░░░▓░░░▒█░░░▒█░░░██░░\n";
        cout << "              █▒▒▒█▒▒▒▒█▒▒▒▓█▒▒▒▓▓▒▒▒▓▓▒▒▒█▓▒▒▒██▒▒▒█▓▒▒▒█▒▒▒▒▓▒▒▒▓▓▒▒▒▓▒░▒▒█▓░░░░░▒░░▓▓▒▒▒▓▓▒▒▒▓▒▒▒▒▓▒▒▒▓█▒▒▓██▒▒\n";
        cout << "              ░▒░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░░░░▒░▒░▒░░░░▒░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒░░▒░░░▒░░░░░░░░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n";
        usleep(150000);
        system("cls");
        cout << "\n\n\n";

        cout << "              \033[38;5;8m▒░░░░░▒▓░░░░░▒▓░░░▒▒░█░░░▒░░█░░▒▒░░█░░▒░░░▓░░▒░░░▓▒░░░░░▒▒░░░░░▒▒░░░░░▒▓░░░░░▒▓░░░░░░█░░░░░░▓░░▒░░░█\n";
        cout << "              ░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▒▒▒▒▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▒▒▒░▒░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒▒▒▒\n";
        cout << "              ▒▒█▒██▓▒█▒▓▓▓▒█▒▒▓█▓█▓▒▒▓█▓▓▓▒▒█▒██▓░▓▒▒▓▓▒▓▒▒▒▓▓▓▓▒▒▒▓▒▓▓▒▒▓░▓▓▓░▓▒▒▒▓▒▓▒░░▒▒▒█▓▒▓█▒██▓▒█▒▓▓▓▒█▒░▒▓\n";
        cout << "              ▓░░░░▓░░░░░▓░░░░▒▓░░░░▓▓░░░░▓▒░░░░▓░░░░▒▓░░░░▒▓░░░░▓▒░░░░█▒░░░░▓░░░░▒▓░░░░▓▓░░░░█▒░░░░█▒░░░░▓░░░░▓▓░\n";
        cout << "              ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n";
        cout << "              ██████████████████████████████████████████████████████████████████████████\033[38;5;21m▒▒\033[38;5;8m███████\033[38;5;21m▒\033[38;5;8m████████████████\n";
        cout << "              ██████████████▓\033[38;5;21m░░░░░░░░░░░░░░░░░░\033[38;5;8m████\033[38;5;21m▒░░\033[38;5;8m█████\033[38;5;21m▒▒▒▒▒▒▒▒\033[38;5;8m██\033[38;5;21m▒▒▒▒▒▒▒▒▒\033[38;5;8m████████████\033[38;5;21m░\033[38;5;8m████▓\033[38;5;21m▒\033[38;5;8m█████████████████\n";
        cout << "              █████████████▓\033[38;5;21m░░\033[38;5;8m█████████\033[38;5;21m░░\033[38;5;8m█████\033[38;5;21m░░\033[38;5;8m▓█\033[38;5;21m▒░░░░\033[38;5;8m███\033[38;5;21m▒▒\033[38;5;8m████████████████\033[38;5;21m▒▒\033[38;5;8m█████████▓\033[38;5;21m░▒▒░░▒▒▒\033[38;5;8m▓\033[38;5;21m▒\033[38;5;8m▓▓██████████████\n";
        cout << "              ██████████████\033[38;5;21m░░░░░░░░\033[38;5;8m▓██\033[38;5;21m░░░░░░░░░\033[38;5;8m█\033[38;5;21m▒░▒\033[38;5;8m█\033[38;5;21m▒░▒\033[38;5;8m██\033[38;5;21m░░\033[38;5;8m███████████\033[38;5;21m▒▒▒▒▒▒▒\033[38;5;8m███████\033[38;5;21m▒░░░░░░░▒▒▒▒▒▒▒▒\033[38;5;8m█████████████\n";
        cout << "              █████████████████████\033[38;5;21m░░\033[38;5;8m██\033[38;5;21m░░\033[38;5;8m███████\033[38;5;21m▒░░░░░░░▒\033[38;5;8m█\033[38;5;21m░░\033[38;5;8m████████████████\033[38;5;21m▒▒\033[38;5;8m███████\033[38;5;21m▒░\033[38;5;8m█\033[38;5;21m░░░▒▒░▒▒▒▒\033[38;5;8m██\033[38;5;21m▒\033[38;5;8m█████████████\n";
        cout << "              ██████████████\033[38;5;21m░░░░░░░░▒\033[38;5;8m██\033[38;5;21m░░\033[38;5;8m███████\033[38;5;21m░░\033[38;5;8m█████\033[38;5;21m░░\033[38;5;8m█▓\033[38;5;21m░░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m███████\033[38;5;21m▒▒\033[38;5;8m█\033[38;5;21m▒▒\033[38;5;8m███████\033[38;5;21m▒\033[38;5;8m██\033[38;5;21m▒\033[38;5;8m█████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ██████████████\033[38;5;21m░░\033[38;5;8m█\033[38;5;21m▒░▒\033[38;5;8m█████\033[38;5;21m░░\033[38;5;8m█▓\033[38;5;21m░▒\033[38;5;8m████████\033[38;5;21m░▒\033[38;5;8m█████\033[38;5;21m▒▒▒▒▒▒\033[38;5;8m▓████\033[38;5;21m▒▒▒▒▒▒▒▒▒▒▒\033[38;5;21m▒▒▒▒▒▒▒\033[38;5;8m█████\033[38;5;21m▒▒▒▒▒▒▒▒\033[38;5;8m████████████\n";
        cout << "              ██████████████\033[38;5;21m░░\033[38;5;8m█\033[38;5;21m░░░░\033[38;5;8m████\033[38;5;21m░░\033[38;5;8m█▓\033[38;5;21m░░\033[38;5;8m███\033[38;5;21m░░\033[38;5;8m██\033[38;5;21m░░░░\033[38;5;8m████\033[38;5;21m░▒\033[38;5;8m▓▓▓▓\033[38;5;21m▒▒▒\033[38;5;8m██\033[38;5;21m▒▒\033[38;5;8m▓▓█▓██▓█▓\033[38;5;21m▒▒\033[38;5;8m████\033[38;5;21m▒▒▒\033[38;5;8m█\033[38;5;21m▒▒▒\033[38;5;8m███████████████████\n";
        cout << "              ██████████████\033[38;5;21m░░\033[38;5;8m█\033[38;5;21m░░\033[38;5;8m▓\033[38;5;21m░░░\033[38;5;8m██\033[38;5;21m░░\033[38;5;8m█▓\033[38;5;21m░▒\033[38;5;8m█\033[38;5;21m▒░░\033[38;5;8m██\033[38;5;21m░░\033[38;5;8m██\033[38;5;21m░░\033[38;5;8m███\033[38;5;21m░░\033[38;5;8m██████\033[38;5;21m░▒\033[38;5;8m█\033[38;5;21m▒▒▒▒▒▒\033[38;5;8m▓███▓\033[38;5;21m▒▒▒▒▒▒\033[38;5;21m▒▒\033[38;5;8m██\033[38;5;21m▒▒▒▒▒▒▒▒\033[38;5;8m██████████████\n";
        cout << "              ██████████████\033[38;5;21m░░\033[38;5;8m█\033[38;5;21m░░\033[38;5;8m▓██\033[38;5;21m░░░░░\033[38;5;8m█▓\033[38;5;21m░░░░\033[38;5;8m▓██\033[38;5;21m░░░░░░░░\033[38;5;8m██\033[38;5;21m░░\033[38;5;8m█████\033[38;5;21m▒▒\033[38;5;8m▓█\033[38;5;21m▒▒\033[38;5;8m████████▓\033[38;5;21m▒\033[38;5;8m▓█▓\033[38;5;21m▒▒▒\033[38;5;8m███████████\033[38;5;21m▒▒\033[38;5;8m████████████\n";
        cout << "              ██████████████\033[38;5;21m░░\033[38;5;8m█\033[38;5;21m░░\033[38;5;8m▓████\033[38;5;21m░░░\033[38;5;8m█▓\033[38;5;21m░░░\033[38;5;8m███\033[38;5;21m░░▒\033[38;5;8m█████\033[38;5;21m░░\033[38;5;8m█\033[38;5;21m░░░░░░░░\033[38;5;8m▓██\033[38;5;21m▒▒▒▒▒▒▒▒▒\033[38;5;8m█▓\033[38;5;21m▒\033[38;5;8m█████\033[38;5;21m▒\033[38;5;8m█\033[38;5;21m▒▒▒▒▒▒▒▒▒\033[38;5;21m▒▒\033[38;5;8m▓████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ███████▓\033[38;5;14m▒▒▒▒\033[38;5;8m▓▓████████████▓▓▓█████▓▓▓███████████████▓\033[38;5;14m▒▒▒▒▒▒\033[38;5;8m█████████████████████\033[38;5;16m▒▒▒▒▒▒▒▒▒\033[38;5;8m███████████\n";
        cout << "              █████\033[38;5;14m▒▒▒▒▒▒▒▒▒▒\033[38;5;8m███████▓\033[38;5;14m▒\033[38;5;8m███\033[38;5;14m▒▒\033[38;5;8m█████\033[38;5;14m▒▒\033[38;5;8m▓███\033[38;5;14m▒\033[38;5;8m███████▓\033[38;5;14m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m████████████\033[38;5;16m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m███████\n";
        cout << "              ███▓\033[38;5;14m▒▒\033[38;5;8m▓██\033[38;5;14m▒▒▒\033[38;5;8m██\033[38;5;14m▒▒▒\033[38;5;8m█████▓\033[38;5;14m▒▒▒▒\033[38;5;8m██\033[38;5;14m▒▒▒▒▒\033[38;5;8m█▓\033[38;5;14m▒▒▒▒\033[38;5;8m██████\033[38;5;14m▒▒▒▒▒\033[38;5;8m███\033[38;5;14m▒▒▒\033[38;5;8m▓███\033[38;5;14m▒▒▒▒▒\033[38;5;8m█████████\033[38;5;16m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m▓█████\n";
        cout << "              ███▓\033[38;5;14m▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m█████▓\033[38;5;14m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m██████\033[38;5;14m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m███████\033[38;5;16m▒▒▒\033[38;5;8m▓▓\033[38;5;16m▒▒▒\033[38;5;8m▓▓\033[38;5;16m▒▒▒\033[38;5;8m▓▓\033[38;5;16m▒▒▒\033[38;5;8m▓▓\033[38;5;16m▒▒▒\033[38;5;8m████\n";
        cout << "              ██████▓\033[38;5;14m▒\033[38;5;8m▓███\033[38;5;14m▒▒\033[38;5;8m██████████\033[38;5;14m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[38;5;8m████████████\033[38;5;14m▒▒▒▒▒▒▒▒▒▒\033[38;5;8m██████████████\033[38;5;16m▒▒▒▒▒\033[38;5;8m███\033[38;5;16m▒▒▒\033[38;5;8m████\033[38;5;16m▒▒▒▒\033[38;5;8m▓█████\n";
        cout << "              █████\033[38;5;14m▒\033[38;5;8m▓██\033[38;5;14m▒▒▒\033[38;5;8m██\033[38;5;14m▒\033[38;5;8m███████████\033[38;5;14m▒\033[38;5;8m████████▓\033[38;5;14m▒\033[38;5;8m████████████\033[38;5;14m▒▒▒\033[38;5;8m▓█\033[38;5;14m▒▒▒\033[38;5;8m▓█\033[38;5;14m▒▒▒\033[38;5;8m▓█████████████▓\033[38;5;16m▒▒\033[38;5;8m█████████████\033[38;5;16m▒▒\033[38;5;8m██████\n";
        cout << "              ███\033[38;5;14m▒▒\033[38;5;8m██\033[38;5;14m▒▒\033[38;5;8m███\033[38;5;14m▒▒\033[38;5;8m█\033[38;5;14m▒▒\033[38;5;8m███████\033[38;5;14m▒▒\033[38;5;8m███████████\033[38;5;14m▒▒\033[38;5;8m▓████████████████████████████████████████████████████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ████████████████████████████████████████████████████████████████████████████████████████████████████\n";
        cout << "              ▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▓▒▒▒▒▒▒▒▒▒▒\n";
        cout << "              ▓░░░▓▒░░░▓░░░▒█░░░▒▓░░░▓▓░░░▓▓░░░▓▓░░░▓▒░░░▓▒░░░█░░░▒▓░░░▒▒░░░▓▓░░░░░░░░▓▓░░░▒▒░░░▓░░░▒█░░░▒█░░░██░░\n";
        cout << "              █▒▒▒█▒▒▒▒█▒▒▒▓█▒▒▒▓▓▒▒▒▓▓▒▒▒█▓▒▒▒██▒▒▒█▓▒▒▒█▒▒▒▒▓▒▒▒▓▓▒▒▒▓▒░▒▒█▓░░░░░▒░░▓▓▒▒▒▓▓▒▒▒▓▒▒▒▒▓▒▒▒▓█▒▒▓██▒▒\n";
        cout << "              ░▒░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░░░░▒░▒░▒░░░░▒░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒░▒░▒░▒░░▒░░░▒░░░░░░░░▒░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\n";
        usleep(150000);
    }
}

void loadingPage()
{
    system("cls");
    cout << "\n\n\n\n\n\n\n\n\n\n\n";
    // cout << "                                \033[38;5;8m╔═════════════════════════════════════════╗\n";
    // cout << "                                ║                                         ║\n";
    // cout << "                                ╚═════════════════════════════════════════╝\n";

    setCursorPosition(35, 12);
    cout << "\033[38;5;21m|_ () /\\ |) | |\\| (_,";

    setCursorPosition(33, 14);
    cout << "🟪"; // 5%
    // setCursorPosition(57,12);
    // cout << "\033[38;5;102m5%";

    usleep(300000);
    // setCursorPosition(34,14);
    cout << "🟪"; // 10%
    // setCursorPosition(57,12);
    // cout << "\033[38;5;102m10%";

    usleep(400000);
    // setCursorPosition(35,14);
    cout << "🟪";
    cout << "🟪";
    cout << "🟪";
    cout << "🟪";
    // setCursorPosition(57,12);
    // cout << "\033[38;5;102m30%";

    usleep(500000); // 30%
    // setCursorPosition(39,14);
    cout << "🟪";
    cout << "🟪";
    cout << "🟪";
    cout << "🟪";
    // setCursorPosition(57,12);
    // cout << "\033[38;5;102m50%";

    usleep(600000); // 50%
    // setCursorPosition(43,14);
    cout << "🟪";
    cout << "🟪";
    cout << "🟪";
    cout << "🟪";
    cout << "🟪";
    cout << "🟪"; // 80%
    // setCursorPosition(57,12);
    // cout << "\033[38;5;102m80%";

    usleep(700000);
    // setCursorPosition(49,14);
    cout << "🟪";
    cout << "🟪"; // 90%
    // setCursorPosition(57,12);
    // cout << "\033[38;5;102m90%";

    usleep(750000);
    // setCursorPosition(51,14);
    cout << "🟪";
    cout << "🟪"; // 100%
    // setCursorPosition(57,12);
    // cout << "\033[38;5;102m100%";

    // this_thread::sleep_for(chrono::seconds(10));
}

// const char *loseMessages[] = {
//     "\033[38;5;224m𝐁𝐞𝐭𝐭𝐞𝐫 𝐥𝐮𝐜𝐤 𝐧𝐞𝐱𝐭 𝐭𝐢𝐦𝐞",
//     "𝗖𝗼𝘂𝗿𝗮𝗴𝗲 𝗺𝗲𝗮𝗻𝘀 𝘁𝗿𝘆𝗶𝗻𝗴 𝗼𝗻𝗲 𝗺𝗼𝗿𝗲 𝘁𝗶𝗺𝗲.",
//     "𝐘𝐨𝐮 𝐝𝐢𝐞𝐝, 𝐛𝐮𝐭 𝐡𝐞𝐲, 𝐲𝐨𝐮 𝐥𝐨𝐨𝐤𝐞𝐝 𝐜𝐨𝐨𝐥 𝐝𝐨𝐢𝐧𝐠 𝐢𝐭",
//     "𝗟𝗲𝗮𝗿𝗻 𝗳𝗿𝗼𝗺 𝗺𝗶𝘀𝘁𝗮𝗸𝗲𝘀 𝗮𝗻𝗱 𝗰𝗼𝗺𝗲 𝗯𝗮𝗰𝗸 𝘀𝘁𝗿𝗼𝗻𝗴𝗲𝗿",
//     "𝐅𝐚𝐢𝐥𝐮𝐫𝐞 𝐢𝐬 𝐧𝐨𝐭 𝐭𝐡𝐞 𝐨𝐩𝐩𝐨𝐬𝐢𝐭𝐞 𝐨𝐟 𝐬𝐮𝐜𝐜𝐞𝐬𝐬; 𝐢𝐭’𝐬 𝐩𝐚𝐫𝐭 𝐨𝐟 𝐭𝐡𝐞 𝐣𝐨𝐮𝐫𝐧𝐞𝐲.",
//     "𝕯𝖔𝖓’𝖙 𝖈𝖗𝖞 𝖇𝖊𝖈𝖆𝖚𝖘𝖊 𝖎𝖙’𝖘 𝖔𝖛𝖊𝖗, 𝖘𝖒𝖎𝖑𝖊 𝖇𝖊𝖈𝖆𝖚𝖘𝖊 𝖞𝖔𝖚 𝖈𝖆𝖓 𝖙𝖗𝖞 𝖆𝖌𝖆𝖎𝖓",
//     "𝐈𝐭’𝐬 𝐧𝐨𝐭 𝐡𝐨𝐰 𝐦𝐚𝐧𝐲 𝐭𝐢𝐦𝐞𝐬 𝐲𝐨𝐮 𝐠𝐞𝐭 𝐤𝐧𝐨𝐜𝐤𝐞𝐝 𝐝𝐨𝐰𝐧; 𝐢𝐭’𝐬 𝐡𝐨𝐰 𝐦𝐚𝐧𝐲 𝐭𝐢𝐦𝐞𝐬 𝐲𝐨𝐮 𝐠𝐞𝐭 𝐛𝐚𝐜𝐤 𝐮𝐩."
// };

// // تعداد جملات
// const int loseMessagesCount = 7;

// void displayRandomLoseMessage()
// {
//     // تولید یک عدد تصادفی برای انتخاب پیام
//     srand(time(0)); // مقداردهی اولیه seed
//     int randomIndex = rand() % loseMessagesCount; // انتخاب ایندکس تصادفی

//     // چاپ پیام انتخابی
//     setCursorPosition(40, 30); // تغییر مکان چاپ پیام در صورت نیاز
//     cout << "\033[38;5;93m" << loseMessages[randomIndex] << "\033[0m"; // نمایش با رنگ دلخواه
// }

void gameOverLogo()
{
    hideCursor();
    setCursorPosition(0, 0);
    // mciSendString(TEXT("stop bgSound"), NULL, 0, NULL);
    // mciSendString(TEXT("close bgSound"), NULL, 0, NULL);
    playLoseSound();

    // setCursorPosition(40, 30);
    // displayRandomLoseMessage();

    // setCursorPosition(0, 0);

    for (int i = 1; i <= 200; i++)
    {
        system("cls");
        cout << "\n\n\n";

        cout << "         \033[38;5;16m███████░   ░███▓   ▒███▓  ████ ░███████░      ███████ ▓██░  ░██▓ ███████▒ ████████\n";
        cout << "        ███░  ▒░░   █████░  ▒████░▓█▓██ ░██▒          ███  ░███ ███  ███░ ███      ███  ▒██░\n";
        cout << "       ░███ ░████  ▓██ ▓██░ ▒██▒█▒██▒██ ░███████     ▒██▒   ███ ░██▒░██░  ███████  ███████░\n";
        cout << "        ███░  ▒██  ████████ ▒██ ███░▒██ ░██▒          ███  ░███  ▒████▓   ███      ███ ▒███\n";
        cout << "         ▓██████░ ███   ▒██▒▒██ ▒██ ▒██ ░███████░      ▓█████▒    ▓███    ███████▓ ███  ▒███\n";

        cout << "                              \033[38;5;231m░░░░░       ▒▒▒▒\n";
        cout << "              ▒▓▓             ░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒           ▓▒   ░▓▒                  ▒\n";
        cout << "            ▓▓▓▓▓▓▓░          ░▒▒░░░▒▒▒▒▒░░░▒▒           ▓▒    ▓▒          ░▒▒ ░▒▒███▒▒░░▒▒\n";
        cout << "        ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓       ░▒▒  ░▒▒▒▒▒░░░▒▒       ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░      ░██ ▒█▓███▓█░░█▓\n";
        cout << "        ▓▓▓▓ ░▓▓▓▒ ▒▓▓▓       ░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒       ▓▓▓▓  ▓▓▓▓ ▒▓▓▓░      ░██ ▒███████░░█▓\n";
        cout << "        ▓▓▓▓▒▒▓▓▓▒▒▓▓▓▓           ░▒▒   ▒▒           ▓▒░▓▓▓▓▓▓▓▓▓▓░▓░      ░███████░█▓████▓\n";
        cout << "        ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓           ░▒▒   ▒▒           ▓▒░▓▓▓▓░▒▓▓▓▓░▓░       ░░░▒███████░░░░\n";
        cout << "        ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓           ░▒▒▒▒▒▒▒                  ▓▒                    ▓█\n";
        cout << "            ▓▓▓▓▓▓▓▒              ░▒▒▒▒▒▒▒               ▒▒▒▓▓▒▒░              ░░░██▒▒▒░\n";
        cout << "          ▓▓       ▒▓░           ░▒░ ░▒  ▒▒                                   ▒▒       ▒\n";
        cout << "        ▓▓           ▒▓          ░▒░ ░▒  ▒▒\n";

        usleep(150000);
        // setCursorPosition(0, 0);
        system("cls");
        cout << "\n\n\n";

        cout << "           \033[38;5;52m███████░   ░███▓   ▒███▓  ████ ░███████░      ███████ ▓██░  ░██▓ ███████▒ ████████\n";
        cout << "          ███░  ▒░░   █████░  ▒████░▓█▓██ ░██▒          ███  ░███ ███  ███░ ███      ███  ▒██░\n";
        cout << "         ░███ ░████  ▓██ ▓██░ ▒██▒█▒██▒██ ░███████     ▒██▒   ███ ░██▒░██░  ███████  ███████░\n";
        cout << "          ███░  ▒██  ████████ ▒██ ███░▒██ ░██▒          ███  ░███  ▒████▓   ███      ███ ▒███\n";
        cout << "           ▓██████░ ███   ▒██▒▒██ ▒██ ▒██ ░███████░      ▓█████▒    ▓███    ███████▓ ███  ▒███\n\n\n";

        cout << "                                \033[38;5;21m░░░░░       ▒▒▒▒\n";
        cout << "                ▒▓▓             ░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒           ▓▒   ░▓▒                  ▒\n";
        cout << "              ▓▓▓▓▓▓▓░          ░▒▒░░░▒▒▒▒▒░░░▒▒           ▓▒    ▓▒          ░▒▒ ░▒▒███▒▒░░▒▒\n";
        cout << "          ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓       ░▒▒  ░▒▒▒▒▒░░░▒▒       ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░      ░██ ▒█▓███▓█░░█▓\n";
        cout << "          ▓▓▓▓ ░▓▓▓▒ ▒▓▓▓       ░▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒       ▓▓▓▓  ▓▓▓▓ ▒▓▓▓░      ░██ ▒███████░░█▓\n";
        cout << "          ▓▓▓▓▒▒▓▓▓▒▒▓▓▓▓           ░▒▒   ▒▒           ▓▒░▓▓▓▓▓▓▓▓▓▓░▓░      ░███████░█▓████▓\n";
        cout << "          ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓           ░▒▒   ▒▒           ▓▒░▓▓▓▓░▒▓▓▓▓░▓░       ░░░▒███████░░░░\n";
        cout << "          ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓           ░▒▒▒▒▒▒▒                  ▓▒                    ▓█\n";
        cout << "              ▓▓▓▓▓▓▓▒              ░▒▒▒▒▒▒▒               ▒▒▒▓▓▒▒░              ░░░██▒▒▒░\n";
        cout << "            ▓▓       ▒▓░           ░▒░ ░▒  ▒▒                                   ▒▒       ▒\n";
        cout << "          ▓▓           ▒▓          ░▒░ ░▒  ▒▒\n";
        usleep(150000);

        if (_kbhit())
        {
            mainMenu();
        }
    }
}

void howToPlayPage()
{
    system("cls");
    cout << "\n\n";
    cout << "\033[38;5;6mHow to Play Space Invaders\n\n";
    cout << "  \033[38;5;8mObjective\n";
    cout << "    \033[38;5;224mYour mission is to protect Earth by destroying all alien invaders before they reach your spaceship!\n ";
    cout << "    Shoot down enemies, avoid their attacks, and survive as long as possible to achieve the highest score.\n\n";
    cout << "  \033[38;5;8mControls\n";
    cout << "    \033[38;5;224mMove Left (A or a): Move your spaceship to the left.\n";
    cout << "    Move Right (D or d): Move your spaceship to the right.\n";
    cout << "    Fire Bullet (Space): Shoot a bullet from your spaceship to attack alien invaders.\n";
    cout << "    Pause Game (P or p): Pause the game and access the pause menu.\n\n";
    cout << "  \033[38;5;8mGameplay\n";
    cout << "    \033[38;5;175m1.Alien Formation\n";
    cout << "            \033[38;5;224mAliens are arranged in rows at the top of the screen.\n";
    cout << "            They will gradually move left, right, and downward as the game progresses.\n";
    cout << "            Destroy them before they reach your spaceship's level.\n";
    cout << "    \033[38;5;175m2.Spaceship Movement\n";
    cout << "            \033[38;5;224mYour spaceship is at the bottom of the screen and can only move horizontally.\n";
    cout << "            Avoid alien projectiles by moving left or right.\n";
    cout << "    \033[38;5;175m3.Shooting\n";
    cout << "            \033[38;5;224mPress the Space key to fire bullets at enemies.\n";
    cout << "            You can fire only one bullet at a time, so aim carefully.\n";
    cout << "    \033[38;5;175m4.Alien Attacks\n";
    cout << "            \033[38;5;224mAliens will fire projectiles downward. If one of these hits your spaceship, you lose a life.\n";
    cout << "    \033[38;5;175m5.Power-Up Enemies\n";
    cout << "            \033[38;5;224mOccasionally, special enemies (like UFOs) will appear. Destroy them for bonus points.\n";
    cout << "    \033[38;5;175m6.Lives\n";
    cout << "            \033[38;5;224mYou start the game with 3 lives.\n";
    cout << "            Losing all lives results in a game over.\n\n";
    cout << "  \033[38;5;8mScoring\n";
    cout << "    \033[38;5;224mPoints are awarded for destroying enemies:\n";
    cout << "            Basic Alien: 10 points\n";
    cout << "            Advanced Alien: 20 points\n";
    cout << "            Elite Alien: 40 points\n";
    cout << "            Bonus UFO: 100 points\n";
    cout << "    Your score is displayed on the screen during the game.\n\n";
    cout << "  \033[38;5;8mGame Over\n";
    cout << "    \033[38;5;224mThe game ends when:\n";
    cout << "            1.All your lives are lost.\n";
    cout << "            2.The aliens reach the bottom of the screen.\n\n";
    cout << "  \033[38;5;8mTips for Success\n";
    cout << "    \033[38;5;224mStay mobile: Don’t remain stationary for too long; keep moving to avoid enemy fire.\n";
    cout << "    Prioritize: Focus on enemies closest to your spaceship to avoid them reaching you.\n";
    cout << "    Look for patterns: Alien movements often follow predictable patterns.\n";
    cout << "    Save bullets: Be precise with your shots to avoid wasting time waiting for a new bullet.\n\n";
    cout << "\033[38;152;mEnjoy the game and defend Earth from the invaders! Good luck, Captain!\n\033[0m";
    usleep(500000);
}

void mainMenu()
{

    system("cls");

    Snowflake snowflakes[SNOWFLAKE_COUNT];

    // مقداردهی اولیه دانه‌های برف
    srand(static_cast<unsigned>(time(0)));
    for (int i = 0; i < SNOWFLAKE_COUNT; i++)
    {
        snowflakes[i].x = rand() % SCREEN_WIDTH;
        snowflakes[i].y = rand() % SCREEN_HEIGHT;
    }

    int situation = 1;
    const int optionsCount = 5;
    bool running = true;

    while (!gameOver)
    {
        system("cls");
        cout << "\n";
        cout << "              \033[38;5;93m███    ███  █████  ██ ███    ██     ███    ███ ███████ ███    ██ ██    ██\n";
        cout << "              ████  ████ ██   ██ ██ ████   ██     ████  ████ ██      ████   ██ ██    ██\n";
        cout << "              ██ ████ ██ ███████ ██ ██ ██  ██     ██ ████ ██ █████   ██ ██  ██ ██    ██\n";
        cout << "              ██  ██  ██ ██   ██ ██ ██  ██ ██     ██  ██  ██ ██      ██  ██ ██ ██    ██\n";
        cout << "              ██      ██ ██   ██ ██ ██   ████     ██      ██ ███████ ██   ████  ██████\n\n\n";

        for (int i = 0; i < 5; i++)
        {
            cout << "                     \033[38;5;8m╭─────────────────────────────────────────────────────╮\n";
            cout << "                     │                                                     │\n";
            cout << "                     ╰─────────────────────────────────────────────────────╯\n\n";
        }

        if (situation == 1)
        {
            setCursorPosition(44, 9);
            cout << "\033[38;5;93mNEW GAME";
        }
        else
        {
            setCursorPosition(44, 9);
            cout << "\033[38;5;110mNEW GAME";
        }
        if (situation == 2)
        {
            setCursorPosition(44, 13);
            cout << "\033[38;5;93mLOADE GAME";
        }
        else
        {
            setCursorPosition(44, 13);
            cout << "\033[38;5;110mLOAD GAME";
        }
        if (situation == 3)
        {
            setCursorPosition(43, 17);
            cout << "\033[38;5;93mHOW TO PLAY";
        }
        else
        {
            setCursorPosition(43, 17);
            cout << "\033[38;5;110mHOW TO PLAY";
        }
        if (situation == 4)
        {
            setCursorPosition(43, 21);
            cout << "\033[38;5;93mLEADER BOARD";
        }
        else
        {
            setCursorPosition(43, 21);
            cout << "\033[38;5;110mLEADER BOARD";
        }
        if (situation == 0)
        {
            setCursorPosition(46, 25);
            cout << "\033[38;5;93mEXIT";
        }
        else
        {
            setCursorPosition(46, 25);
            cout << "\033[38;5;110mEXIT";
        }

        if (_kbhit())
        {
            char input = _getch();

            if (input == 's' || input == 'S')
            {
                situation = dealWithNegatives(situation + 1, optionsCount);
                playSelectionSound(); // پخش صدا
                // situation++;
            }
            else if (input == 'w' || input == 'W')
            {
                situation = dealWithNegatives(situation - 1, optionsCount);
                playSelectionSound(); // پخش صدا
                // situation--;
            }

            else if (input == '\n' || input == '\r')
            {
                if (situation == 1)
                {
                    running = false;
                    system("cls");
                    cout << "enter your name :";
                    cin >> userName;
                    resetGameState();

                    start();
                }
                else if (situation == 3)
                {
                    howToPlayPage();
                    getch();
                    continue;
                }
                else if (situation == 4)
                {
                    leaderBoard();
                }

                else if (situation == 0)
                {
                    //stopBackgroundSound();
                    // mciSendString(TEXT("stop bgSound"), NULL, 0, NULL);
                    // mciSendString(TEXT("close bgSound"), NULL, 0, NULL);
                    running = false;
                    abort();
                }
            }
            if (situation < 0)
                situation = dealWithNegatives(situation, 5);

            situation %= 5;
        }
        // پاک کردن دانه‌های برف قبلی
        clearSnowflakes(snowflakes);

        // به‌روزرسانی و رسم دانه‌های برف
        updateSnowflakes(snowflakes);
        drawSnowflakes(snowflakes);

        // تأخیر برای کنترل سرعت حلقه
        // ؟؟؟؟؟  this_thread::sleep_for(chrono::milliseconds(100));
        usleep(1000);

        // if (input == '\n' || input == '\r')
        //{
        //  switch (situation)
        //  {
        //      case 1:
        //          newGame();
        //          break;
        //  }
        //}
        //}
    }
}

void gameChar(int x)
{
    if (x == 0)
        cout << "  ";
    if (x == -1)
        cout << "🐙";
    if (x == -2)
        cout << "👽";
    if (x == -3)
        cout << "🤖";
    if (x == 1)
        cout << "👾";
    if (x == 2)
        cout << "🛸";
    if (x == 3)
        cout << "↟ ";
    if (x == 4)
        cout << "⬜️";
    if (x == 5)
        cout << "↡ ";
    if (x == 10)
        cout << "\u2554"; // Left-up-corner
    if (x == 11)
        cout << "\u2550"; // Horizontal
    if (x == 12)
        cout << "\u2557"; // Right-up-corner
    if (x == 13)
        cout << "\u2551"; // Vertical
    if (x == 14)
        cout << "\u255A"; // Left-down-corner
    if (x == 15)
        cout << "\u255D"; // Right-down-corner
}

void chap()
{

    for (int i = 0; i <= 25; i++)
    {
        for (int j = 0; j <= 100; j += 2)
        {
            if (gameArr[i][j] != 5)
            {
                setCursorPosition(j, i);
                gameChar(gameArr[i][j]);
            }
            else
            {
                setCursorPosition(j, i);
                gameChar(gameArr[i][j]);
            }
        }
        cout << '\n';
    }
    setCursorPosition(110, 40);
    cout << "Score : " << point << ' ';
    setCursorPosition(110, 41);
    cout << "lives : " << lives << ' ';
    setCursorPosition(110, 42);
    cout << "time : " << int(double(clock() - startTime) / CLOCKS_PER_SEC);
}

void pre()
{
    // gameArr[0][0] = 10;
    // for (int i = 1; i < 100; i++)
    //     gameArr[0][i] = 11;
    // gameArr[0][100] = 12;
    // for (int i = 0; i <= 100; i+= 100) {
    //     for (int j = 1; j < 25; j++)
    //         gameArr[j][i] = 13;
    // }
    // gameArr[25][0] = 14;
    //   for (int i = 1; i < 100; i++)
    //     gameArr[25][i] = 11;
    // gameArr[25][100] = 15;

    // gameArr[3][6] = 1;
    // gameArr[3][10] = 1;
    gameArr[18][22] = 4;
    gameArr[18][20] = 4;
    gameArr[18][18] = 4;
    gameArr[19][22] = 4;
    gameArr[19][20] = 4;
    gameArr[19][18] = 4;

    gameArr[18][42] = 4;
    gameArr[18][40] = 4;
    gameArr[18][38] = 4;
    gameArr[19][42] = 4;
    gameArr[19][40] = 4;
    gameArr[19][38] = 4;

    gameArr[18][62] = 4;
    gameArr[18][60] = 4;
    gameArr[18][58] = 4;
    gameArr[19][62] = 4;
    gameArr[19][60] = 4;
    gameArr[19][58] = 4;

    gameArr[18][82] = 4;
    gameArr[18][80] = 4;
    gameArr[18][78] = 4;
    gameArr[19][82] = 4;
    gameArr[19][80] = 4;
    gameArr[19][78] = 4;

    gameArr[24][52] = 2;
    gameArr[24][50] = 2;
    gameArr[24][48] = 2;
    gameArr[23][50] = 2;

    for (int i = 1; i <= 4; i++)
    {
        if (i >= 3)
        {

            for (int j = 1; j < 9; j++)
            {
                gameArr[i][2 * j] = 1;
            }
        }

        else if (i == 1)
        {

            for (int j = 1; j < 9; j++)
            {
                gameArr[i][2 * j] = -3;
            }
        }

        else
        {

            for (int j = 1; j < 9; j++)
            {
                gameArr[i][2 * j] = -2;
            }
        }
    }
}
// 100x25
void moveToRight()
{
    for (int i = 1; i <= loc; i++)
    {
        for (int j = 98; j >= 2; j -= 2)
        {
            if (gameArr[i][j] != 3 && gameArr[i][j] != 5 && gameArr[i][j - 2] != 3 && gameArr[i][j - 2] != 5)
                swap(gameArr[i][j], gameArr[i][j - 2]);
        }
    }
}

void moveToLeft()
{
    for (int i = 1; i <= loc; i++)
    {
        for (int j = 0; j < 99; j += 2)
        {
            if (gameArr[i][j] != 3 && gameArr[i][j] != 5 && gameArr[i][j + 2] != 3 && gameArr[i][j + 2] != 5)
                swap(gameArr[i][j], gameArr[i][j + 2]);
        }
    }
}

void moveToDown()
{
    for (int i = loc; i >= 1; i--)
    {
        for (int j = 0; j <= 99; j++)
        {
            if (gameArr[i][j] != 3 && gameArr[i][j] != 5 && gameArr[i + 1][j] != 3 && gameArr[i + 1][j] != 5)
                swap(gameArr[i + 1][j], gameArr[i][j]);
        }
    }
}

bool move()
{
    if (redDirect == 0)
    {
        for (int j = 98; j >= 2; j -= 2)
        {
            swap(gameArr[0][j], gameArr[0][j - 2]);
        }
        if (gameArr[0][98] != 0)
        {
            redDirect = 1;
        }
    }
    else
    {
        for (int j = 0; j <= 96; j += 2)
        {
            swap(gameArr[0][j], gameArr[0][j + 2]);
        }
        if (gameArr[0][0] != 0)
        {
            gameArr[0][0] = 0;
        }
    }

    for (int i = 1; i <= 25; i++)
    {
        for (int j = 0; j <= 99; j += 2)
        {
            if (gameArr[i][j] == 3)
            {
                if (i == 1)
                {
                    if (gameArr[0][j] == -1)
                    {
                        point += 100;
                        gameArr[0][j] = 0;
                    }
                    gameArr[i][j] = 0;
                    bullet = false;
                }
                else
                {
                    if (gameArr[i - 1][j] != 0)
                    {
                        if (gameArr[i - 1][j] == 1)
                            point += 10;
                        if (gameArr[i - 1][j] == -2)
                            point += 20;
                        if (gameArr[i - 1][j] == -3)
                            point += 40;

                        gameArr[i - 1][j] = 0;
                        gameArr[i][j] = 0;
                        bullet = false;
                        turnTime *= 0.95;
                    }
                    else
                    {
                        swap(gameArr[i][j], gameArr[i - 1][j]);
                    }
                }
            }
        }
    }

    for (int i = 25; i >= 1; i--)
    {
        for (int j = 0; j <= 99; j += 2)
        {
            if (gameArr[i][j] == 5)
            {

                if (i == 25)
                {
                    gameArr[i][j] = 0;
                    bullet = false;
                }

                else
                {
                    if (gameArr[i + 1][j] != 0)
                    {
                        if (gameArr[i + 1][j] == 2)
                        {
                            lives--;
                            setCursorPosition(60, 45);
                            cout << "You lost a live!";
                            usleep(5000);

                            gameArr[i][j] = 0;

                            return false;
                        }
                        gameArr[i + 1][j] = 0;
                        gameArr[i][j] = 0;
                    }
                    else
                    {
                        swap(gameArr[i][j], gameArr[i + 1][j]);
                    }
                }
            }
        }
    }
    if (direction == 1)
    {
        moveToRight();
        for (int i = 0; i <= 25; i++)
        {
            if (gameArr[i][98] == 1)
            {
                direction = 2;
                lastDirection = 1;
                break;
            }
        }
    }
    else if (direction == 3)
    {
        moveToLeft();
        for (int i = 0; i <= 25; i++)
        {
            if (gameArr[i][0] == 1)
            {
                direction = 2;
                lastDirection = 3;
                break;
            }
        }
    }
    else if (direction == 2)
    {
        for (int i = 0; i <= 99; i += 2)
        {
            if (gameArr[loc - 1][i] == 1)
            {
                gameOver = true;
                break;
            }
        }
        moveToDown();
        direction = 4 - lastDirection;
        lastDirection = 2;
    }

    for (int j = 0; j <= 99; j += 2)
    {
        if (gameArr[25][j] == 1)
            return false;
    }
    return true;
}

void pauseGame()
{
    system("cls");
    int input;
    cin >> input;
    if (input == 1)
        return;
    // save and Leave
    if (input == 2)
        return;
    // continueGame
    if (input == 3)
        return;

    // just leave
}

void start()
{
    resetGameState();
    startTime = clock();

    playBackgroundSound();

    srand(time(nullptr));
    pre();
    while (lives != 0)
    {
        if (_kbhit())
        {
            char inp = getch();
            if (inp == ' ' && bullet == false)
            {
                gameArr[spaceX - 2][spaceY] = 3;
                bullet = true;
            }
            else if ((inp == 'd' || inp == 'D') && spaceY <= 96)
            {

                swap(gameArr[spaceX][spaceY - 2], gameArr[spaceX][spaceY + 4]);
                swap(gameArr[spaceX - 1][spaceY + 2], gameArr[spaceX - 1][spaceY]);
                spaceY += 2;
            }
            else if ((inp == 'a' || inp == 'A') && spaceY >= 4)
            {

                swap(gameArr[spaceX][spaceY - 4], gameArr[spaceX][spaceY + 2]);
                swap(gameArr[spaceX - 1][spaceY - 2], gameArr[spaceX - 1][spaceY]);
                spaceY -= 2;
            }
            else if (inp == 'p' || inp == 'P')
            {
                //BackgroundSound(); // توقف صدا
                // mciSendString(TEXT("stop bgSound"), NULL, 0, NULL);
                // mciSendString(TEXT("close bgSound"), NULL, 0, NULL);
                pauseGame();
                playBackgroundSound(); // بعد از خروج از Pause صدا ادامه پیدا کنه
            }
        }

        chap();
        usleep(turnTime );
        // system("cls");
        setCursorPosition(0, 0);
        if (turn % 3 == 0)
        {
            move();
            int random2 = rand() % 3;
            if (random2 == 2 && (turn % 12 == 0))
            {
                gameArr[0][0] = -1;
                redDirect = 0;
            }

            if (turn % 10 == 0)
            {
                int i, j;
                for (i = 25; i >= 0; i--)
                {
                    for (j = 0; j <= 99; j += 2)
                    {
                        if (gameArr[i][j] == 1 || (gameArr[i][j] <= -1 && gameArr[i][j] >= -3))
                            break;
                    }
                    if (gameArr[i][j] == 1 || (gameArr[i][j] <= -1 && gameArr[i][j] >= -3))
                        break;
                }
                while (true)
                {
                    int random = rand() % 50;
                    if (gameArr[i][2 * random] == 1 || (gameArr[i][2 * random] <= -1 && gameArr[i][2 * random] >= -3))
                    {
                        gameArr[i + 2][2 * random] = 5;
                        break;
                    }
                }
            }
        }
        turn++;
    }
    // mciSendString(TEXT("stop bgSound"), NULL, 0, NULL);
    // mciSendString(TEXT("close bgSound"), NULL, 0, NULL);
    //stopBackgroundSound();

    endTime = clock();
    elapsed = double(endTime - startTime) / CLOCKS_PER_SEC;

    players[users].name = userName;
    players[users].score = point;
    players[users].time = elapsed;
    users++;

    ofstream userInfoFile2("winningData.txt");

    //useful();
    
    userInfoFile2 << users  << endl;
    for (int i = 0; i < users; i++)
    {
        userInfoFile2 << players[i].name << ' ' << players[i].score << ' ' << players[i].time << endl;
        // userInfoFile2 << players[i].name << ' ';
        // userInfoFile2 << players[i].score << ' ';
        // userInfoFile2 << players[i].time << ' ';
        // userInfoFile2 << endl;
    }
    userInfoFile2.close();
    //leaderBoard();
    // userInfoFile2 << userName << ' ';
    // userInfoFile2 << point << ' ';
    // userInfoFile2 << elapsed << ' ';
    
    
    system("cls");
    setCursorPosition(25, 25);
    gameOverLogo();
}

int main()
{
    firstAppearLogo();
    usleep(900000);
    hideCursor();
    loadingPage();
    usleep(790000);
    mainMenu();
    hideCursor();
    start();
}
