#include <bits/stdc++.h>

using namespace std;

int main(int argc, char const *argv[])
{
    vector <int> v1= {3,4,2,1, 1};
    vector <int> v2= {4, 1, 2, 4, 2, 1,1, 0};
    sort(v1.begin(), v1.end());
    unordered_set<int> s(v2.begin(), v2.end());
    int intersection = count_if(v1.begin(), v1.end(), [&](int k) {return s.find(k) != s.end();});
    cout << "calculated intersection: "<<intersection<<endl;
    return 0;
}

