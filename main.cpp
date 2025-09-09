#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <conio.h>

int main()
{
    std::ios_base::sync_with_stdio(false);
    std::cout.tie(nullptr);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    int n,m;
    long long SEED = 0;
    std::cin >>n >>m;
    std::cout<<"Please enter seed \n";
    std::cin >>SEED;
    int Labir[n][m];
    srand(SEED);
    for(int i=0;i<n;i++) //создание основания мира
    {
        for(int j=0;j<m;j++)
        {
            Labir[i][j] = rand()%10;
        }
    }
    for(int i=0;i<n;i++) //заполнения мира
    {
        for(int j=0;j<m;j++)
        {
            if((Labir[i][j-1]+Labir[i][j]) >= 7)
            {
                Labir[i][j] = 0;
            }
            else
            {
                Labir[i][j] = 1;
            }
        }
    }
    for(int i=0;i<n;i++) //исправления мира
    {
        for(int j=0;j<m;j++)
        {
            if((Labir[i][j]+Labir[i][j+1]+Labir[i][j+2]+Labir[i+1][j]+Labir[i+1][j+1])+Labir[i+2][j] >= 4)
            {
                Labir[i][j] = 0;
            }
            else
            {
                Labir[i][j] = 1;
            }
        }
    }
    for(int i=0;i<n;i++)//доработка мира
    {
        for(int j=0;j<m;j++)
        {
            if((Labir[i-1][j]+Labir[i-1][j-1]+Labir[i-1][j+1]+Labir[i][j-1]+Labir[i][j+1]+Labir[i+1][j]+Labir[i+1][j-1]+Labir[i][j] <= 1))
                Labir[i][j] = 0;
            if((Labir[i][j+1] && Labir[i-1][j]) == 1)
                Labir[i][j+1] == 0;
            else
            {
                if((Labir[i][j] && Labir[i+1][j+1]) == 1)
                    Labir[i][j] == 0;
            }
            if((Labir[i][j] == 0) && ((Labir[i-1][j] && Labir[i][j+1] && Labir[i][j-1] && Labir[i+1][j]) == 1))
                Labir[i][j] = 1;
            if((Labir[i][j] == 0) && ((Labir[i-1][j] && Labir[i][j+1] && Labir[i][j-1] && Labir[i+1][j] && Labir[i+1][j-1]) == 1))
                Labir[i][j] = 1;
            if((Labir[i][j] == 1) && (Labir[i][j+1] == 0) && (Labir[i+1][j] == 0) && (Labir[i+1][j+1] == 1))
                Labir[i+1][j+1] = 0;
        }
    }
    for(int i=0;i<5;i++)//начальная локация
    {
        for(int j=m-5;j<m;j++)
        {
            Labir[i][j] = 0;
        }
    }
    bool Point = false;
    int X,Y;
    srand(time(NULL));
    while(Point != true)//появления кровати
    {
        int x = rand()%n, y = rand()%m;
        int Summ = 0;
        for(int i=y-2;i<y+2;i++)
        {
            for(int j=x-2;j<x+2;j++)
            {
                Summ+=Labir[i][j];
            }
        }
        if(Summ == 0)
        {
            X=x;
            Y=y;
            Point = true;
        }
    }
    Labir[Y][X] = 2;
    /*for(int i=0;i<n;i++) //Ходьба вверх\вниз
    {
        for(int j=0;j<m;j++)
        {
            if (Labir[i][j] == 2)
            {
                std::cout<<"++";
            }
            else
            {
            if (Labir[i][j] == 0)
                std::cout<<"░░";
            else
            {
                if(Labir[i][j+1] == 0 || Labir[i+1][j] == 0 || Labir[i+1][j+1] == 0)
                    std::cout<<"▓▓";
                else
                    std::cout<<"██";
            }
            }
        }
        std::cout<<"\n";
    }
    /*std::cout<<"\n\n\n\n";
     for(int i=0;i<n;i++)
    {
        for(int j=0;j<m;j++)
        {
          std::cout<<Labir[i][j];
        }
        std::cout<<"\n";
    }
      for(int j=0;j<n;j++) //Ходьба влево\вправо
    {
        for(int i=0;i<m;i++)
        {
            if (Labir[i][j] == 2)
            {
                std::cout<<"++";
            }
            else
            {
            if (Labir[i][j] == 0)
                std::cout<<"░░";
            else
            {
                if(Labir[i][j+1] == 0 || Labir[i+1][j] == 0 || Labir[i+1][j+1] == 0)
                    std::cout<<"▓▓";
                else
                    std::cout<<"██";
            }
            }
        }
        std::cout<<"\n";
    }*/
    //std::cout<<"\n\n\n\n";
char c;
int S = 2, H = n - 3;

while(c != ' ')
{
    system("cls");
    for(int i = S-4; i < S+4; i++)
    {
        for(int j = H-6; j < H+6; j++)
        {
            if(i == S && j == H)
            {
                std::cout << "TT";
            }
            else
            {
                if(Labir[i][j] == 0)
                    std::cout << "░░";
                /*else
                {
                    if((Labir[i][j+1] == 0 || Labir[i+1][j] == 0 || Labir[i+1][j+1] == 0) && Labir[i][j] == 1)
                        std::cout << "▓▓";
                    else
                        std::cout << "██";
                }*/
                if(Labir[i][j] == 1)
                     std::cout << "██";
            }
        }
        std::cout << "\n"; // замените на std::endl для сброса буфера
    }

    // Принудительно сбросить буфер вывода
    std::cout << std::flush;

    c = _getch();
    switch(c)
    {
   case 'w':
       if(Labir[S-1][H] == 1 || S<0)
        break;
       S--;
       break;
   case 's':
       if(Labir[S+1][H] == 1 || S>n)
        break;
       S++;
       break;
   case 'a':
       if(Labir[S][H-1] == 1 || H<0)
        break;
       H--;
       break;
   case 'd':
       if(Labir[S][H+1] == 1 || H>m)
        break;
       H++;
       break;
    }
}
    return 0;
}
