#include<bits/stdc++.h>

using namespace std;

int main(int argc, char const *argv[])
{
    cout<<"Syntax test bench\n";
    vector <int> v1 = {1,2,3,4};
    vector <int> v2 = {4,3,1,2};
    map <set <int>, bool > present_set;
    present_set.insert(make_pair(set <int> (v1.begin(), v1.end()), true));
    cout<<present_set.count(set <int> (v1.begin(), v1.end()))<<endl;
    cout<<present_set.count(set <int> (v2.begin(), v2.end()))<<endl;

    return 0;
}

