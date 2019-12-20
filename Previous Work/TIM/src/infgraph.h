
class VV
{
    public:
        vector<int> head;
        vector<int> next;
        vector<int> data;
        vector<int> vsize;
        void clear()
        {
            head.clear();
            next.clear();
            data.clear();
            vsize.clear();
        }
        // trick for not change code
        void push_back(vector<int> x){
            ASSERT(x.size()==0);
            addVector();
        }
        void addVector()
        {
            head.push_back(-1);
            vsize.push_back(0);
        }
        int size(int t){
            return vsize[t];
        }
        //vv[a].push_back(b)
        void addElement( int a, int b)
        {
            //a.push_back(b);
            vsize[a]++;
            data.push_back(b);
            next.push_back(head[a]);
            head[a]=next.size()-1;

        }
        //loop
        //for(int t:vv[a])
        //for (int x=vv.head[a]; x!=-1; x=vv.next[x])
        //{
        //t=vv.data[x]
        //}
};

#include <queue>	
#include <utility> 

struct CompareBySecond {
	bool operator()(pair<int, int> a, pair<int, int> b)
	{
		return a.second < b.second;
	}
};


class InfGraph:public Graph
{
    public:
        vector<vector<int>> hyperG;
        //VV hyperG;
        vector<vector<int>> hyperGT;

        InfGraph(string folder, string graph_file):Graph(folder, graph_file){
            hyperG.clear();
            for(int i=0; i<n; i++)
                hyperG.push_back(vector<int>());
            for(int i=0; i<12; i++)
                sfmt_init_gen_rand(&sfmtSeed, i+1234);
        }

        enum ProbModel {TR, WC, TR001};
        ProbModel probModel;

        void BuildHypergraphR(int64 R){
            hyperId=R;
            //for(int i=0; i<n; i++)
                //hyperG[i].clear();
            hyperG.clear();
            for(int i=0; i<n; i++)
                hyperG.push_back(vector<int>());
            hyperGT.clear();
            while((int)hyperGT.size() <= R)
                hyperGT.push_back( vector<int>() );

            for(int i=0; i<R; i++){
                BuildHypergraphNode(sfmt_genrand_uint32(&sfmtSeed)%n, i, true);
            }
            int totAddedElement=0;
            for(int i=0; i<R; i++){
                for(int t:hyperGT[i])
                {
                    hyperG[t].push_back(i);
                    //hyperG.addElement(t, i);
                    totAddedElement++;
                }
            }
            ASSERT(hyperId == R);
        }

        int BuildHypergraphNode(int uStart, int hyperiiid, bool addHyperEdge){
            int n_visit_edge=1;
            if(addHyperEdge)
            {
                ASSERT((int)hyperGT.size() > hyperiiid);
                hyperGT[hyperiiid].push_back(uStart);
            }

            int n_visit_mark=0;
            //for(int i=0; i<12; i++) ASSERT((int)visit[i].size()==n);
            //for(int i=0; i<12; i++) ASSERT((int)visit_mark[i].size()==n);
            //hyperiiid ++;
            q.clear();
            q.push_back(uStart);
            ASSERT(n_visit_mark < n);
            visit_mark[n_visit_mark++]=uStart;
            visit[uStart]=true;
            while(!q.empty()) {
                int expand=q.front();
                q.pop_front();
                if(influModel==IC){
                    int i=expand;
                    for(int j=0; j<(int)gT[i].size(); j++){
                        //int u=expand;
                        int v=gT[i][j];
                        n_visit_edge++;
                        double randDouble=double(sfmt_genrand_uint32(&sfmtSeed))/double(RAND_MAX)/2;
                        if(randDouble > probT[i][j])
                            continue;
                        if(visit[v])
                            continue;
                        if(!visit[v])
                        {
                            ASSERT(n_visit_mark < n);
                            visit_mark[n_visit_mark++]=v;
                            visit[v]=true;
                        }
                        q.push_back(v);
                        //#pragma omp  critical 
                        //if(0)
                        if(addHyperEdge)
                        {
                            //hyperG[v].push_back(hyperiiid);
                            ASSERT((int)hyperGT.size() > hyperiiid);
                            hyperGT[hyperiiid].push_back(v);
                        }
                    }
                }
                else if(influModel==LT){
                    if(gT[expand].size()==0)
                        continue;
                    ASSERT(gT[expand].size()>0);
                    n_visit_edge+=gT[expand].size();
                    double randDouble=double(sfmt_genrand_uint32(&sfmtSeed))/double(RAND_MAX)/2;
                    for(int i=0; i<(int)gT[expand].size(); i++){
                        ASSERT( i< (int)probT[expand].size());
                        randDouble -= probT[expand][i];
                        if(randDouble>0)
                            continue;
                        //int u=expand;
                        int v=gT[expand][i];

                        if(visit[v])
                            break;
                        if(!visit[v])
                        {
                            visit_mark[n_visit_mark++]=v;
                            visit[v]=true;
                        }
                        q.push_back(v);
                        if(addHyperEdge)
                        {
                            ASSERT((int)hyperGT.size() > hyperiiid);
                            hyperGT[hyperiiid].push_back(v);
                        }
                        break;
                    }
                }
                else
                    ASSERT(false);
            }
            for(int i=0; i<n_visit_mark; i++)
                visit[visit_mark[i]]=false;
            return n_visit_edge;
        }

        //return the number of edges visited
        int64 hyperId = 0;
        deque<int> q;
        sfmt_t sfmtSeed;
        //set<int> seedSet;
		vector<int> seedSet;

		double BuildSeedSet()
		{

			priority_queue<pair<int, int>, vector<pair<int, int>>, CompareBySecond>heap;
			vector<int>coverage(n, 0);

			for (int i = 0; i < n; i++)
			{
				pair<int, int>tep(make_pair(i, (int)hyperG[i].size()));
				heap.push(tep);
				coverage[i] = (int)hyperG[i].size();
			}

			int maxInd;

			long long influence = 0;
			long long numEdge = hyperGT.size();

			// check if an edge is removed
			vector<bool> edgeMark(numEdge, false);
			// check if an node is remained in the heap
			vector<bool> nodeMark(n + 1, true);

			seedSet.clear();
			while ((int)seedSet.size()<k)
			{
				pair<int, int>ele = heap.top();
				heap.pop();
				if (ele.second > coverage[ele.first])
				{
					ele.second = coverage[ele.first];
					heap.push(ele);
					continue;
				}

				maxInd = ele.first;
				vector<int>e = hyperG[maxInd];  //the edge influence
				influence += coverage[maxInd];
				seedSet.push_back(maxInd);
				nodeMark[maxInd] = false;

				for (unsigned int j = 0; j < e.size(); ++j){
					if (edgeMark[e[j]])continue;

					vector<int>nList = hyperGT[e[j]];
					for (unsigned int l = 0; l < nList.size(); ++l){
						if (nodeMark[nList[l]])coverage[nList[l]]--;
					}
					edgeMark[e[j]] = true;
				}
			}
			return 1.0*influence / hyperGT.size()*n;
		}


















};



