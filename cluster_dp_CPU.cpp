
// Clustering by fast search and find of density peaks
// Science 27 June 2014:
// Vol. 344 no. 6191 pp. 1492-1496
// DOI: 10.1126/science.1242072
// http://www.sciencemag.org/content/344/6191/1492.full//


#include "iostream"
#include <stdio.h>
//#include <ctime>
#include "vector"
#include "math.h"
#include "algorithm"
using namespace std;

#define DIM 3
#define elif else if

#ifndef bool
#define bool int
#define false ((bool)0)
#define true  ((bool)1)
#endif

#define NEIGHBORRATE 0.020

#define RHO_RATE 0.6
#define DELTA_RATE 0.2

vector<vector<double> > data;
vector< vector<double> > data_distance;
   // vector<int> near_cluster_label;
    //vector<bool> cluster_halo;
vector<double> rho;
vector<double> delta;
vector<int> decision;
int nSamples;
struct Point3d {
    double x;
    double y;
    Point3d(double xin, double yin) : x(xin), y(yin) {}
};

int dataPro(vector< vector<double> > &src, vector<Point3d> &dst){
    for (int i = 0; i < src.size(); i++){
        Point3d pt(src[i][0], src[i][1]);
        dst.push_back(pt);
    }
    return dst.size();
}

double get_point_Distance(Point3d &pt1, Point3d &pt2){
    double tmp = pow(pt1.x - pt2.x, 2) + pow(pt1.y - pt2.y, 2);
    return pow(tmp, 0.5);
}

void get_distanc(vector< vector<double> > &data_distance, vector<Point3d> &data){
    int data_size = data.size();
    for (int i = 0; i < data_size; ++i)
    {
        /* code */
        vector<double> tmp(data_size, 0.0);
        for (int j = 0; j < data_size; ++j)
        {
            /* code */
            if (i != j)
            {
                /* code */
                tmp[j] = get_point_Distance(data[i], data[j]);
            }
        }
        data_distance.push_back(tmp);
    }
}

void sort(vector<double> &v, long left, long right){
    if (left < right){
        double key = v[left];
        long low = left;
        long high = right;
        while (low < high) {
            // high下标位置开始，向左边遍历，查找不大于基准数的元素
            while (low < high && v[high] >= key) {
                high--;
            }
            if (low < high) {// 找到小于准基数key的元素
                v[low] = v[high];// 赋值给low下标位置，low下标位置元素已经与基准数对比过了
                low++;// low下标后移
            }
            else {// 没有找到比准基数小的元素
                // 说明high位置右边元素都不小于准基数
                break;
            }
            // low下标位置开始，向右边遍历，查找不小于基准数的元素
            while (low < high && v[low] <= key) {
                low++;
            }
            if (low < high) {// 找到比基准数大的元素
                v[high] = v[low];// 赋值给high下标位置，high下标位置元素已经与基准数对比过了
                high--;// high下标前移，
            }
            else {// 没有找到比基准数小的元素
                // 说明low位置左边元素都不大于基准数
                break;
            }
        }
        v[low] = key;// low下标赋值基准数
        sort(v, left, low - 1);
        sort(v, low + 1, right);
    }
}
double getdc(vector< vector<double> > &data_distance, double neighborRate,int nSamples){
    int nSamples_rate = round(nSamples*(nSamples - 1)*neighborRate / 2);
    double dc = 0.0;
    vector<double> distance_tmp;

    for (int i = 0; i <nSamples; ++i)
    {
        /* code */
        for (int j = i + 1; j < nSamples; j++)
        {
            /* code */
            distance_tmp.push_back(data_distance[i][j]);
        }
    }

    sort(distance_tmp, 0, distance_tmp.size()-1);//sort

    dc = distance_tmp.at(nSamples_rate);
    return dc;
}

//cut-off kernel
vector<double> getLocalDensity(vector< vector<double> > &data_distance, double dc,int nSamples){
    vector<double> rho(nSamples, 0.0);
    for (int i = 0; i < nSamples - 1; i++){
        for (int j = i + 1; j < nSamples; j++){
            if (data_distance[i][j] < dc){
                ++rho[i];
                ++rho[j];
            }
        }
        //cout<<"getting rho. Processing point No."<<i<<endl;
    }
    return rho;
}
//gussian kernel
vector<double> getLocalDensity_gussian(vector< vector<double> > &data_distance, double dc, int nSamples){
    vector<double> rho(nSamples, 0.0);
    for (int i = 0; i < nSamples - 1; i++){
        for (int j = i + 1; j < nSamples; j++){
            rho[i] = rho[i] + exp(-pow((data_distance[i][j] / dc), 2));
            rho[j] = rho[j] + exp(-pow((data_distance[i][j] / dc), 2));
        }
        //cout<<"getting rho. Processing point No."<<i<<endl;
    }
    return rho;
}
vector<double> getDistanceToHigherDensity(vector< vector<double> > &data_distance, vector<double> &rho, vector<int> &near_cluster_label){
    int nSamples = data_distance[0].size();
    vector<double> delta(nSamples, 0.0);
    for (int i = 0; i < nSamples; i++){
        double dist = 0.0;
        bool flag = false;
        near_cluster_label.push_back(-1);
        for (int j = 0; j < nSamples; j++){
            if (i == j) continue;
            if (rho[j] > rho[i]){
                double tmp = data_distance[i][j];
                if (!flag){
                    dist = tmp;
                    near_cluster_label.back() = j;
                    flag = true;
                }
                else {
                    dist = tmp < dist ? tmp : dist;
                    near_cluster_label.back() = j;
                }
            }
        }
        if (!flag){
            for (int j = 0; j < nSamples; j++){
                double tmp = data_distance[i][j];
                dist = tmp > dist ? tmp : dist;
            }
            near_cluster_label.back() = 0;//the bigger data's lable will step over later
        }
        delta[i] = dist;
        cout<<i<<":"<<near_cluster_label.back()<<endl;
    }
    return delta;
}
//应该讲rho进行排序，避免有相同最大密度的点，也可以通过高斯核计算密度来最大程度避免这个问题

/*
vector<int> decidegragh(vector<double> &delta, vector<double> &rho){
int nSamples = rho.size();
vector<int> decision(nSamples, -1);
vector<double> multiple(nSamples, 0.0);
for (int i = 0; i < nSamples; ++i)
{

multiple[i] = delta[i] * rho[i];
    }
    for (int i = 0; i < CLUSTER_NUM; ++i)
    {

        double tmp_max = 0.0;
        int tmp_lable = 0;
        for (int j = 0; j < nSamples; ++j)
        {

            if (tmp_max <= multiple[j])
            {

                tmp_max = multiple[j];
                tmp_lable = j;
            }

        }
        multiple[tmp_lable] = 0.0;
        decision[tmp_lable] = i;
    }
    return decision;
}

*/

vector<int> decidegragh(vector<double> &delta, vector<double> &rho,int &cluster_num){
    int nSamples = rho.size();
    int counter = 0;
    vector<int> decision(nSamples, -1);
    double max_rho = 0.0, min_rho = 0.0, max_delta = 0.0, min_delta = 0.0,rho_bound=0.0,delta_bound=0.0;
    for (int i = 0; i < nSamples; ++i)
    {
        /* code */
        if (max_rho <= rho[i])
        {
            max_rho = rho[i];
        }
        if (min_rho>=rho[i])
        {
            min_rho = rho[i];
        }
        if (max_delta <= delta[i])
        {
            max_delta = delta[i];
        }
        if (min_delta >= delta[i])
        {
            min_delta = delta[i];
        }
    }
    rho_bound = RHO_RATE*(max_rho - min_rho) + min_rho;
    delta_bound = DELTA_RATE*(max_delta - min_delta) + min_delta;
    for (int i = 0; i < nSamples; ++i)
    {
        /* code */
        if (rho[i]>rho_bound && delta[i]>delta_bound)
        {
            decision[i] = counter;
            counter++;
        }
    }
    cluster_num = counter + 1;
    return decision;
}

void quicksort(vector<double> &rho, vector<int> &rho_order, long left, long right){
    if (left < right){
        long key = rho_order[left];
        long low = left;
        long high = right;
        while (low < high){
            while (low < high && rho[rho_order[high]] <= rho[key]){
                high--;
            }
            if (low<high)
            {
                rho_order[low] = rho_order[high];
                low++;
            }
            else
            {
                break;
            }
            while (low < high && rho[rho_order[low]] >= rho[key]){
                low++;
            }
            if (low<high)
            {
                rho_order[high] = rho_order[low];
                high--;
            }
            else
            {
                break;
            }
        }
        rho_order[low] = key;
        quicksort(rho, rho_order, left, low - 1);
        quicksort(rho, rho_order, low + 1, right);
    }
}
void assign_cluster(vector<double> &rho, vector<int> &decision, vector<int> &near_cluster_label){
    vector<int> rho_order(rho.size(), -1);
    for (int i = 0; i < rho.size(); ++i)
    {
        /* code */
        rho_order[i] = i;
    }
    quicksort(rho, rho_order, 0, rho.size()-1);
    for (int i = 0; i < rho.size(); ++i)
    {
        /* code */
        printf("%d:%f  ",rho_order[i],rho[rho_order[i]] );
    }
    for (int i = 0; i < rho_order.size(); ++i)
    {
        /* code */
        if (decision[rho_order[i]] == -1)
        {
            /* code */
            decision[rho_order[i]] = decision[near_cluster_label[rho_order[i]]];
        }
    }


}
int assign_cluster_recursive(int index){
    double min_dist=0;
    bool flag=true;
    int neighbor=-1;
    for(int i=0;i<nSamples;i++){
        if(flag){
            min_dist=data_distance[index][i];
            flag=false;
        }
        if(!flag){
            if(min_dist>data_distance[index][i]&&rho[index]<rho[i]){
                min_dist=data_distance[index][i];
                neighbor=i;
            }

        }

    }
    if(decision[neighbor]==-1)
        decision[neighbor]=assign_cluster_recursive(neighbor);
}
void get_halo(vector<int> &decision, vector< vector<double> > &data_distance, vector<bool> &cluster_halo, vector<double> &rho, double dc,int cluster_num){
    vector<double> density_bound(cluster_num, 0.0);
    int nSamples = decision.size();
    for (int i = 0; i < nSamples - 1; ++i)
    {
        /* code */
        double avrg_rho;
        for (int j = i+1; j < nSamples; ++j)
        {
            /* code */
            if (decision[i] != decision[j] && data_distance[i][j]<dc)
            {
                /* code */
                avrg_rho = (rho[i] + rho[j]) / 2;
                if (avrg_rho>density_bound[decision[i]])
                {
                    /* code */
                    density_bound[decision[i]] = avrg_rho;
                }
                if (avrg_rho>density_bound[decision[j]])
                {
                    /* code */
                    density_bound[decision[j]] = avrg_rho;
                }
            }
        }
    }
    for (int i = 0; i < nSamples; ++i)
    {
        /* code */
        if (rho[i] <= density_bound[decision[i]])
        {
            /* code */
            cluster_halo.push_back(false);
        }
        else cluster_halo.push_back(true);
    }
}
int main(int argc, char** argv)
{
    //long start, end;
    //errno_t err;
    FILE *input;
    if((input=fopen("dataset_2D.txt", "r"))==NULL)
        printf("data file not found\n");
    else
    {
        printf("data file was opened\n");
    }

    double point_x, point_y;
    int point_lable;
    int counter = 0,cluster_num=0;


    while (1){
        if (fscanf(input, "%lf,%lf,%d", &point_x, &point_y, &point_lable) == EOF) break;

        vector<double> tpvec;
        data.push_back(tpvec);

        data[counter].push_back(point_x);
        data[counter].push_back(point_y);

        ++counter;
    }
    if (fclose(input) == 0)
        printf("read %d samples,datafile closed\n", counter);
    else
    {
        printf("datafile closed failed\n");
    }

    //start = clock();
    cout << "********" << endl;
    vector<Point3d> points;
    nSamples=dataPro(data, points);
    get_distanc(data_distance, points);
    double dc = getdc(data_distance, NEIGHBORRATE,nSamples);
    rho = getLocalDensity_gussian(data_distance, dc, nSamples);
    delta = getDistanceToHigherDensity(data_distance, rho, near_cluster_label);
    decision = decidegragh(delta, rho,cluster_num);
    //assign_cluster(rho, decision, near_cluster_label);
    for(int i=0;i<nSamples;i++){
        assign_cluster_recursive(i);
    }
    //get_halo(decision, data_distance, cluster_halo, rho, dc, cluster_num);

    //end = clock();
    //cout << "used time: " << ((double)(end - start)) / CLOCKS_PER_SEC << endl;

    FILE *output;
    if((output=fopen("result_CPU.txt", "w"))!=NULL)
        printf("result file open");
    for (int i = 0; i < counter; ++i)
    {
        /* code */
        fprintf(output, "%4.2f,%4.2f,%d\n", data[i][0], data[i][1], decision[i]);
    }
    fclose(output);

    return 0;
}


