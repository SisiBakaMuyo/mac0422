#include <bits/stdc++.h>

using namespace std;

int main (int argc, char * argv[]) {
	ifstream file;
	string filePath, filename, buffer, netUsage;
    double cpuUsage = 0;
    int measurements = 0;

    if(argc < 3) {
        fprintf(stderr,"Usage: %s <Path> <Filename>\n",argv[0]);
        exit(EXIT_FAILURE);
    }

	filePath = argv[1];
    filename = argv[2];

    file.open(filePath + '/' + filename);
    if(!file.is_open()) {
        cerr << "Error opening the file!" << endl;
        exit(1);
    }

    while(getline(file, buffer)) {
        int cpuLength = buffer.find('%');
        cpuUsage += stod(buffer.substr(0, cpuLength));
        
        int separatorChar = buffer.find('|', cpuLength);
        netUsage = buffer.substr(separatorChar + 2);
        measurements++;
    }
	file.close();
	
	cpuUsage /= measurements;

    ofstream output(filePath + "/agregate-" + filename, ios::trunc);
    output.precision(5);
    output << cpuUsage << "% | " + netUsage; 
    output.close();
    return 0;
} 
