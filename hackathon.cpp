#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <queue>
#include <set>
using namespace std;
int times;
queue<double> q;
double Max;
double max(double a, double b){
    if(a >= b) return a;
    return b;
}
int abs(int x){
    if(x < 0) return -x;
    return x;
}
double double_abs(double x){
    if(x >= 0) return x;
    return -x;
}
double equals_to_zero(double x){
    if(double_abs(x) <= 0.00000001) return 0.0;
    return x;
}
struct data{
    string ID;
    double price, size, size_in_quote;
    string trade_time;
    bool Can_buy;
    bool ignore;
};

struct item_data{
    double price, size;
};

multiset<item_data> item;

bool operator<(item_data a, item_data b){
    return a.price < b.price;
}

vector<data> trade;
double amount;
double profit;
double money;
double total_fee;
double total_buy_quote;

double size(int index){
    return (double)(trade[index].size);
}

double price(int index){
    return (double)(trade[index].price);
}

double slippage(int index)
{
    srand( time(NULL) );
    double positive_effect=0.5;
    double fortune=0;
    double value=0;
    positive_effect+=  ( 0.5- 0.0001) * rand() / (RAND_MAX + 1.0) + 0.0001;
    fortune+=( 1- 0.0001) * rand() / (RAND_MAX + 1.0) + 0.0001;
     if(fortune>=positive_effect)
     {
         double x = (double) rand() / (RAND_MAX + 1.0); //0~1

         if(x>=0.5)
         {
             value=1;
         }
         else if(x>0.5 && 0.7>x )
         {
             value=0.99;
         }
         else if(x>0.7 && 0.8>x )
         {
             value=0.98;
         }
         else
         {
             value=0.97;
         }

     }
     else
    {
        double x = (double) rand() / (RAND_MAX + 1.0);

         if(x>=0.5)
         {
             value=1;
         }
         else if(x>0.5 && 0.7>x )
         {
             value=1.01;
         }
         else if(x>0.7 && 0.8>x )
         {
             value=1.02;
         }
         else
         {
             value=1.03;
         }
     }
     return(double)(value);
}

double additional_fee()
{
    int vip_level=1;
    double fee_rate=0.00075;
    while(vip_level<9)
    {
        if(total_buy_quote/vip_level>1000000)
        {
            vip_level++;
            fee_rate-=0.000075;
        }
        else break;
    }


    return(double)(fee_rate);
}

void buy(int index){
    double p = price(index) * slippage(index);
    total_buy_quote += p * size(index);
    money -= p * size(index);
    money -= additional_fee() * p * size(index);
    total_fee += additional_fee() * p * size(index);
    amount += size(index);
    times ++;
}

void sell(double now_price, double Size){   // (slippage(index) * price(now_loc), Size)
    money += Size * now_price;
    amount -= Size;
    total_fee += additional_fee() * now_price * Size;
}

void get_data(int m){
    int i;
    fstream file;
    file.open("BTCUSDT-trades-2022-10-19.csv");
    string line;
    int loc = 1;
    while(getline(file, line) && loc <= m){
        string S = "";
        istringstream tmp(line);
        string s;
        while(getline(tmp, s, ',')){
            S += s;
            S += " ";
        }
        stringstream ss;
        ss << S;
        string ID;
        double price, size, size_in_quote;
        string trade_time, Can_buy, ignore;
        ss >> ID;
        ss >> price;
        ss >> size;
        ss >> size_in_quote;
        ss >> trade_time;
        ss >> Can_buy;
        ss >> ignore;
        trade.push_back({ID, price, size, size_in_quote, trade_time, (Can_buy == "True" ? false : true), true});
        loc ++;
    }
    file.close();
}

void period(int n, int m){
    int now_loc = 0;
    double w_sum = 0;
    double w = 1;
    double plus = 0.1;
    double sum = 0.0, weighted_sum = 0.0, last;
    double last_price = 0.0;
    last = w;
    while(now_loc < n){
        now_loc ++;
        weighted_sum += w * price(now_loc);
        q.push(w);
        w_sum += w;
        w += plus;
        plus += 0.1;
    }
    while(now_loc < m){
        Max = max(Max, abs(money));
        now_loc ++;
        if(now_loc == m) last_price = price(now_loc);
        double price_base = weighted_sum / (double)w_sum;


        if(trade[now_loc].Can_buy && price(now_loc) <= price_base){  // buy
            buy(now_loc);
            item.insert({price(now_loc), size(now_loc)});
        }
        else if(!trade[now_loc].Can_buy && price(now_loc) >= price_base){

            if(amount <= size(now_loc)){ // I can sell all items I have
                sell(slippage(now_loc) * price(now_loc), amount);
            }
            else { // Can only sell partial items
                sell(slippage(now_loc) * price(now_loc), size(now_loc));
            }
        }
        // sell or buy



        weighted_sum += w * price(now_loc);
        double last_w = q.front();
        q.pop();
        q.push(w);
        weighted_sum -= last_w * price(now_loc - n);
        w_sum += w;
        w += plus;
        w_sum -= last_w;
        plus += 0.1;
    }
    money += slippage(m) * amount * last_price;
    total_fee += additional_fee() * last_price * amount;
}
/*
void up (int n,int m,int k,int l)
{
    auto it = item.begin();
    for(int i=1;i<=n;i=k)
    {
        double remain_size = size(i);
        for(int j=i+1;j<i+m+1;j++)
        {
            if(price(i+1)-price(i)<=0)
            {
                k=i;
                break;
            }
            else
            {
                buy(i);
                item.insert({price(i), size(i)});
                k+=m+l;
                sell(k, );
            }
        }
    }
}
void down (int n,int m,int k,int l) //n = size of data, m = consecutive decrease amount, k = next node, l = 過多少時間買進
{
    auto it = item.begin();
    for(int i=1;i<=n;i=k)
    {
        double remain_size = size(i);
        for(int j=i+1;j<i+m+1;j++)
        {
            if(price(i+1)-price(i)>=0)
            {
                k=i;
                break;
            }
            else
            {
                sell(i);
                item.insert({price(i), size(i)});
                k+=m+l;
            }
        }
    }
}
 */



void period_SMA(int n, int m, double up_pointer, double down_pointer){
    queue<double> dif_queue;
    int i, now_loc = 1;
    double last_price = price(now_loc);
    double up_ratio_sum = 0, down_ratio_sum = 0;
    while(now_loc <= n){
        now_loc ++;
        double dif = price(now_loc) - last_price;
        double dif_ratio = dif / last_price;
    //    cout << fixed << setprecision(10) << dif << " / " << last_price << " = " << dif_ratio << '\n';
        dif_queue.push(dif_ratio);
        if(price(now_loc) >= last_price) up_ratio_sum += dif_ratio;
        else down_ratio_sum += double_abs(dif_ratio);
        last_price = price(now_loc);
        // calculate initial RSI
    }
    bool If_Sell = false, If_Buy = false;
    while(now_loc < m){
        now_loc ++;
        double dif = price(now_loc) - last_price;
        double dif_ratio = dif / last_price;
        double up_ratio_ave = up_ratio_sum / (double)n;
        double down_ratio_ave = down_ratio_sum / (double)n;
        double RSI = equals_to_zero((up_ratio_ave) / (up_ratio_ave + down_ratio_ave));
   //     cout << fixed << setprecision(5) <<  " RSI = " << RSI << '\n';

        // update RSI
        if(price(now_loc) >= last_price) up_ratio_sum += dif_ratio;
        else down_ratio_sum += double_abs(dif_ratio);
        dif_queue.push(dif_ratio);


        double oldest_ratio = dif_queue.front();
        dif_queue.pop();
        if(oldest_ratio >= 0) up_ratio_sum -= oldest_ratio;
        else down_ratio_sum -= double_abs(oldest_ratio);

        last_price = price(now_loc);


        if(If_Buy){
            if(trade[now_loc].Can_buy){
                buy(price(now_loc));
                If_Buy = false;
            }
            else continue;
        }  // (slippage(index) * price(now_loc), Size)
        else if(If_Sell){
            if(!trade[now_loc].Can_buy){
                if(amount <= size(now_loc)){ // I can sell all items I have
                    sell(slippage(now_loc) * price(now_loc), amount);
                    If_Sell = false;
                }
                else { // Can only sell partial items
                    sell(slippage(now_loc) * price(now_loc), size(now_loc));
                }
            }
        }
        else {
            if(RSI <= down_pointer) {
                If_Buy = true;
                if(trade[now_loc].Can_buy){
                    buy(price(now_loc));
                    If_Buy = false;
                }
                else continue;
            }
            else if(RSI >= up_pointer && amount > 0.0) {
                If_Sell = true;
                if(!trade[now_loc].Can_buy){
                    if(amount <= size(now_loc)){ // I can sell all items I have
                        sell(slippage(now_loc) * price(now_loc), amount);
                        If_Sell = false;
                    }
                    else { // Can only sell partial items
                        sell(slippage(now_loc) * price(now_loc), size(now_loc));
                    }
                }
            }
        }
    }
    money += slippage(m) * amount * last_price;
    total_fee += additional_fee() * last_price * amount;
}

void _up(int n,int m){ //n is the big range, m is the small range
    int k = 2;
    double remain_size = 0, buy_price = 0;
    bool checksell = 0;
    for(int i = 1; i <= n; i = k){
        if(!checksell){
        //    cout << "1\n";
            bool change = false, counter = true;
            for(int j = i + 1; j < i + m + 1; j++){
                if(j == n){
                    k = n;
                    change = true;
                    counter = false;
                    break;
                }
                if(price(j) - price(j - 1) <= 0){
                    k = j;
                    change = true;
                    counter = false;
                    break;
                }
            }
            if(counter){
                buy(k);
                buy_price = price(k);
                checksell = 1;
                remain_size = size(k);
                k++;
                continue;
            }
            else if(!change) k++;
        }//buy
        if(checksell && trade[i].Can_buy){
            k ++;
        }
        if(checksell && !trade[i].Can_buy){
            if(remain_size > 0.0){
                if(remain_size >= size(i)){
                    sell(slippage(i) * price(i), size(i));
                    remain_size -= size(i);
                }
                else{
                    sell(slippage(i) * price(i), remain_size);
                    remain_size = 0;
                    checksell = 0;
                }
            }
            k ++;
        }
    }
    sell(n, remain_size);
}

int main(){

    amount = 0;
    Max= 0;
    profit = 0.0;
    money = 0.0;
    total_buy_quote = 0.0;
    total_fee = 0.0;
    trade.push_back({"", 0.0, 0.0, 0.0, "", "false", "false"});
    get_data(100000);
    item.clear();
 //   period_SMA(5, 10000, 0.55, 0.45);
 //   _up(10000, 6);
    period(1000, 10000);
    cout << fixed << setprecision(5) << "money = " << money << '\n';
    cout << "buying times = " << times << '\n';
    cout << "total buy quote = " << total_buy_quote << '\n';
    cout << "total fee = " << total_fee << '\n';
}


