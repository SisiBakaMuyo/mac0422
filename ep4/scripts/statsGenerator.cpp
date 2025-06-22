#include <bits/stdc++.h>

using namespace std;

#define NUM_MEASURES 100

class Stats {
    public:
        double cpuUsage;
        pair<double, double>netUsage;
        Stats() {
            cpuUsage = 0;
            netUsage = make_pair(0, 0);
        }
        Stats(double cpu, pair<double, double>net) {
            cpuUsage = cpu;
            netUsage = net;
        }
};

int findValueLength(string src) {
    int index;
    for(index = 0; index < src.length(); index++) {
        if((src[index] >= 'a' && src[index] <= 'z') ||
            (src[index] >= 'A' && src[index] <= 'Z'))
            break;
    }

    return index;
}

double getValueInKb(string src, int length) {
    double value = stod(src.substr(0, length));
    string type = src.substr(length);

    if(type == "MB")
        return value*1e3;
    else if(type == "B")
        return value/1e3;
    return value;
}

void printConfidenceInterval(ofstream& file, double average, double varianceEstimator) {
    double leftSide = average - 1.984*varianceEstimator;
	double rightSide = average + 1.984*varianceEstimator;
    file.precision(5);
	file << average << " | [" << leftSide << ", " << rightSide << "]" << endl;
}

int main (int argc, char * argv[]) {
	ifstream file;
	string folder, buffer, netUsageString, inputNet, outputNet;
    Stats accStats;

    if(argc < 2) {
        fprintf(stderr,"Usage: %s <Folder>\n",argv[0]);
        exit(EXIT_FAILURE);
    }

	folder = argv[1];
	
    Stats *values = new Stats[NUM_MEASURES];

	for (int i = 1; i <= NUM_MEASURES; i++) {
        Stats currentStats;
		file.open(folder + "/agregate-results-" + to_string(i) + ".txt");

        if(!file.is_open()) {
            cerr << "Error opening the file!" << endl;
            exit(1);
        }
		getline(file, buffer);
        int cpuLength = buffer.find('%');
        currentStats.cpuUsage = stod(buffer.substr(0, cpuLength));

        int separatorChar = buffer.find('|', cpuLength);
        netUsageString = buffer.substr(separatorChar + 2);

        separatorChar = netUsageString.find('/');
        inputNet = netUsageString.substr(0, separatorChar - 1);
        outputNet = netUsageString.substr(separatorChar + 2);
        
        currentStats.netUsage.first = getValueInKb(inputNet, findValueLength(inputNet));
        currentStats.netUsage.second = getValueInKb(outputNet, findValueLength(outputNet));

        accStats.cpuUsage += currentStats.cpuUsage;
        accStats.netUsage.first += currentStats.netUsage.first;
        accStats.netUsage.second += currentStats.netUsage.second;

        values[i - 1] = currentStats;
        file.close();
	}

    accStats.cpuUsage /= NUM_MEASURES;
    accStats.netUsage.first /= NUM_MEASURES;
    accStats.netUsage.second /= NUM_MEASURES;

	// variance estimator: s = sqrt(1 / (n - 1) * sum_1^n(X[i] - mean))
	Stats varianceEstimator;
	for (int i = 1; i <= NUM_MEASURES; i++) {
		varianceEstimator.cpuUsage += pow(values[i - 1].cpuUsage - accStats.cpuUsage, 2);
		varianceEstimator.netUsage.first += pow(values[i - 1].netUsage.first - accStats.netUsage.first, 2);
		varianceEstimator.netUsage.second += pow(values[i - 1].netUsage.second - accStats.netUsage.second, 2);
    }

	varianceEstimator.cpuUsage = sqrt(varianceEstimator.cpuUsage);
	varianceEstimator.netUsage.first = sqrt(varianceEstimator.netUsage.first);
	varianceEstimator.netUsage.second = sqrt(varianceEstimator.netUsage.second);

	varianceEstimator.cpuUsage *= (1/sqrt(NUM_MEASURES-1));
	varianceEstimator.netUsage.first *= (1/sqrt(NUM_MEASURES-1));
	varianceEstimator.netUsage.second *= (1/sqrt(NUM_MEASURES-1));

    ofstream output(folder + "/confidence-interval.txt", ios::trunc);
    printConfidenceInterval(output, accStats.cpuUsage, varianceEstimator.cpuUsage);
    printConfidenceInterval(output, accStats.netUsage.first, varianceEstimator.netUsage.first);
    printConfidenceInterval(output, accStats.netUsage.second, varianceEstimator.netUsage.second);
    output.close();
    delete[] values;
    return 0;
} 
