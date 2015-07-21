#include "workload.hpp"
#include<stdlib.h>

namespace mwg {
    workload::workload(YAML::Node &yamlNodes) {
        if (!yamlNodes) {
            cerr << "Workload constructor and !nodes" << endl;
            exit(EXIT_FAILURE);
            }
        if (!yamlNodes.IsSequence()) {
            cerr << "Not sequnce in workload type initializer" << endl;
            exit(EXIT_FAILURE);
        }
        
        for (auto yamlNode : yamlNodes) {
            if (!yamlNode.IsMap()) {
                cerr << "Node in workload is not a yaml map" << endl;
                exit(EXIT_FAILURE);
            }
            if (yamlNode["type"].Scalar() == "query" ) {
                auto mynode = make_shared<query> (yamlNode);
                nodes[yamlNode["name"].Scalar()] = mynode;
                // this is an ugly hack for now
                vectornodes.push_back(mynode);
                //cout << "In workload constructor and added query node" << endl;
            }
            else if (yamlNode["type"].Scalar() == "insert") {
                auto mynode = make_shared<insert> (yamlNode);
                nodes[yamlNode["name"].Scalar()] = mynode;
                // this is an ugly hack for now
                vectornodes.push_back(mynode);
                //cout << "In workload constructor and added insert node" << endl;
            }
            else
                cerr << "Don't know how to handle workload node with type " << yamlNode["type"] << endl;
        }

        // link the things together
        for (auto mnode : vectornodes) {
            mnode->nextNode = nodes[mnode->nextName];
            if (mnode->nextNode) {
                ;//cout << "Successfully set mnode->next for " << mnode->name << endl;
            }
            else
                cerr << "Failed to set mnode->next for " << mnode->name << endl;
        }

    }
    
    void workload::execute(mongocxx::client &conn) {
        //        for (auto mnode : vectornodes) {
        //    mnode->execute(conn);}
        vectornodes[0]->executeNode(conn);
    }
}
