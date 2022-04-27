#include "Five.h"
#include "Notification.h"
#include "Manager.h"
#include "command_classes/CommandClass.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <math.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <thread>
#include <fstream>
#include <vector>

#define PORT 5101

using namespace OpenZWave;
using namespace Five;
using namespace std;

//Returns the node object whose ID you've given
NodeInfo* Five::getNode(uint8 nodeID, list<NodeInfo*> *nodes) {
    list<NodeInfo*>::iterator it;
    for (it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->m_nodeId == nodeID) {
            return (*it);
        }
    }
    return NULL;
}

//Creates a node object containing all the informations about the node whose ID you've given
NodeInfo Five::getNodeConfig(uint32 homeId, uint8 nodeId, list<NodeInfo*> *nodes)
{
    list<NodeInfo*>::iterator it;
    NodeInfo node;

    node.m_homeId = homeId;
    node.m_name = Manager::Get()->GetNodeProductName(node.m_homeId, nodeId);
    node.m_nodeId = nodeId;
    node.m_nodeType = Manager::Get()->GetNodeType(node.m_homeId, nodeId);
    for(it = nodes->begin(); it != nodes->end(); ++it)
    {
        if((*it)->m_nodeId == nodeId)
        {
            node.m_values = (*it)->m_values;
        }
    }

    for (int i = 0; i < 29; i++) {
		uint8 bite{ 0 };
		uint8 *bitmapPixel = &bite;
		node.m_neighbors[i] = bitmapPixel;
	}

    if (nodeId != 1)
    {
        Manager::Get()->GetNodeNeighbors(homeId, nodeId, node.m_neighbors);
    }
    
    return node;

}

/// A value has changed on the Z-Wave network and this is a different value.
bool Five::valueChanged(Notification const* notification, list<NodeInfo*> *nodes) {
    bool state;
    bool* ptr = &state;

    ValueID v{ notification->GetValueID() };
    string output{ "[VALUE CHANGED]\n" };
    list<NodeInfo*>::iterator it;

    for (it = nodes->begin(); it != nodes->end(); it++) {
        if ((*it)->m_nodeId == v.GetNodeId()) {
            (*it)->m_values.push_back(v);
        }
    }

    output += "id           : " + int(v.GetId()) + '\n';
    output += "index        : " + int(v.GetIndex()) + '\n';
    output += "type         : " + v.GetTypeAsString() + '\n';
    output += "node_id      : " + int(v.GetNodeId()) + '\n';
    output += "node_name    : " + Manager::Get()->GetNodeProductName(v.GetHomeId(), v.GetNodeId()) + '\n';
    output += "node_type    : " + int(Manager::Get()->GetNodeDeviceType(v.GetHomeId(), v.GetNodeId())) + '\n';
    output += "node_stage   : " + Manager::Get()->GetNodeQueryStage(v.GetHomeId(), v.GetNodeId()) + '\n';
    output += "node_version : " + int(Manager::Get()->GetNodeVersion(v.GetHomeId(), v.GetNodeId())) + '\n';
    output += "node_specific: " + Manager::Get()->GetNodeSpecificString(v.GetHomeId(), v.GetNodeId()) + '\n';
    output += "node_label   : " + Node(v.GetHomeId(), v.GetNodeId()).GetInstanceLabel(v.GetCommandClassId(), v.GetInstance()) + '\n';
    output += "cc_id        : " + int(v.GetCommandClassId()) + '\n';
    output += "name         : " + Manager::Get()->GetCommandClassName(v.GetCommandClassId()) + '\n';

    if (v.GetType() == ValueID::ValueType_Bool) {
        Manager::Get()->GetValueAsBool(v, ptr);
        output += "value_as_bool: " + *ptr + '\n';
    }

    // return output;
    return true;
}

//Allows to turn on or off any object who has a switch parameter
bool Five::setSwitch(ValueID valueId, bool state) {   
    string answer;
    cout << "true(1) or false(0) ?" << endl;
	cin >> answer;

    if(answer=="true" || answer=="True" || answer == "1"){
        state = true;
    }else if(answer == "false" || answer == "False" || answer == "0"){
        state = false;
    }

    Manager::Get()->SetValue(valueId, state);
    return true;
}


//Allows to set the value of an intensity type value
bool Five::setIntensity(ValueID valueId, IntensityScale intensity) {
    Manager::Get()->SetValue(valueId, to_string(intensity));
    return true;
}

//Allows to change the color of a light by giving an hexadecimal value
bool Five::setColor(ValueID valueId)
{
    cout << "Enter hexadecimal color" << endl;
    string hexColor;
    cin >> hexColor;
    Manager::Get()->SetValue(valueId, hexColor);
    return true;
}

//Allows to change the value of a list type value by displaying all possible values
bool Five::setList(ValueID valueId){
    vector<string> vectS;
    vector<string> *vectSPtr = &vectS;
    int counter{0};
    string s;
    string* ptr = &s;
    cout << valueId.GetTypeAsString() << endl;
    Manager::Get()->GetValueListItems(valueId, vectSPtr) ;
    vector<string>::iterator it;
    int choice;
    string response;
    Manager::Get()->GetValueListSelection(valueId, ptr);
    cout << "Current Value: " << s << endl;
    for(it = vectS.begin(); it != vectS.end(); ++it){
        counter++;
        cout << counter << ". " << (*it) << endl;
    }
    cout << "Choose your poison" << endl;
    cin >> response;
    choice = stoi(response);
    cout << choice << endl;
    counter = 0;
    for(it = vectS.begin(); it != vectS.end(); ++it){
        counter++;
        if(choice == counter){
            cout << (*it) << endl;
            Manager::Get()->SetValueListSelection(valueId, (*it));
        }
    }
    //Manager::Get()->SetValue(valueId, UnitName);
    return true;
}

//Allows to set the intensity of a volume parameter
bool Five::setVolume(ValueID valueId, IntensityScale intensity){
    Manager::Get()->SetValue(valueId, to_string(intensity));
    return true;
}

//Allows to set the value of a duration parameter
bool Five::setDuration(ValueID valueId) {
    string response;
    cout << "Please enter a duration in seconds:" << endl;
    cin >> response;
    Manager::Get()->SetValue(valueId, response);
    return true;
}

//Allows to set the value of a int parameter
bool Five::setInt(ValueID valueId){
    string response;
    cout << "Please enter a value in Int:" << endl;
    cin >> response;
    Manager::Get()->SetValue(valueId, stoi(response));
    return true;
}

//Allows to set the value of a boolean parameter
bool Five::setBool(ValueID valueId)
{
    string answer;
    bool state(0);
    cout << "true(1) or false(0) ?" << endl;
	cin >> answer;
	if(answer == "true" || answer == "True" || answer == "1"){
        state = true;
    }else if(answer == "false" || answer == "False" || answer == "0"){
        state = false;
    }

    Manager::Get()->SetValue(valueId, state);
    return true;
}

//Allows to push or release a button parameter
bool Five::setButton(ValueID valueId){
    string input;
    cout << "Press a key to push button" << endl;
    cin >> input;
    Manager::Get()->PressButton(valueId);
    cout << "Press a key to release button" << endl;
    cin >> input;
    Manager::Get()->ReleaseButton(valueId);
    return true;
}

// Create a new NodeInfo.
NodeInfo* Five::createNode(Notification const* notification) {
	uint32 homeId = notification->GetHomeId();
	uint8 nodeId = notification->GetNodeId();
	ValueID valueID = notification->GetValueID();
	string name = Manager::Get()->GetNodeProductName(homeId, nodeId);
	string type = valueID.GetTypeAsString();


	NodeInfo *n = new NodeInfo();
	n->m_homeId		= homeId;
	n->m_nodeId		= nodeId;
	n->m_name     	= name;
	n->m_nodeType 	= type;

    cout << "createnode" << endl;
    for (int i = 0; i < 29; i++) {
		uint8 bite{ 0 };
		uint8 *bitmapPixel = &bite;
		n->m_neighbors[i] = bitmapPixel;
	}

    //if (nodeId != 1)
    //{
   
    //}


	return n;
}

// Check if <nodeID> exists in <nodes>.
bool Five::isNodeNew(uint8 nodeID, list<NodeInfo*> *nodes) {
	list<NodeInfo*>::iterator it;
	for (it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
			return false;
		}
	}
	return true;
}

//Returns the number of dead nodes in <nodes>
int Five::deadNodeSum(list<NodeInfo*> *nodes) {
	int counter{ 0 };
	list<NodeInfo*>::iterator it;
	for (it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_isDead && (*it)->m_nodeId != 1) {
			counter++;
		}
	}
	return counter;
}

//Returns the number of alive nodes in <nodes>
int Five::aliveNodeSum(list<NodeInfo*> *nodes) {
	return (nodes->size() - 1) - deadNodeSum(nodes);
}

//Check if type <needle> is in list <haystack>
bool Five::containsType(Notification::NotificationType needle, vector<Notification::NotificationType> haystack) {
    for (int i = 0; i < int(haystack.capacity()); i++) {
        if (needle == haystack[i]) {
            return true;
        }
    }
    return false;
}

//Check if node <needle> is in list <haystack>
bool Five::containsNodeID(uint8 needle, list<NodeInfo*> haystack) {
    list<NodeInfo*>::iterator it;
    for (it = haystack.begin(); it != haystack.end(); it++) {
        if ((*it)->m_nodeId == needle) {
            return true;
        }
    }
    return false;
}

// Refresh members in the oldNodeInfo thanks to this valueID.
void Five::refreshNode(ValueID valueID, NodeInfo* oldNodeInfo) {
	oldNodeInfo->m_homeId = valueID.GetHomeId();
	oldNodeInfo->m_name = Manager::Get()->GetNodeName(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_nodeType = Manager::Get()->GetNodeType(valueID.GetHomeId(), valueID.GetNodeId());
	oldNodeInfo->m_values.push_back(valueID);
}

//Takes the node linked to <notification>, and adds it to <node>
void Five::pushNode(Notification const *notification, list<NodeInfo*> *nodes) {
	NodeInfo* n{ createNode(notification) };
	uint8 nodeID{ notification->GetNodeId() };

	if (isNodeNew(nodeID, nodes)) {
		nodes->push_back(n);
	}
}

//Takes the node linked to <notification>, and removes it from <node>
void Five::removeNode(Notification const *notification, list<NodeInfo*> *nodes) {
    uint8 nodeID{ notification->GetNodeId() };
	list<NodeInfo*>::iterator it;
    NodeInfo* n;

	for (it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
            n = (*it);
			nodes->remove(n);
            return;
		}
	}
}

//Adds a value to a node's values list
bool Five::addValue(ValueID valueID, NodeInfo *node) {
	bool isNew{ true };

    list<ValueID>::iterator it;
    for (it = node->m_values.begin(); it != node->m_values.end(); it++) {
        if ((*it).GetId() == valueID.GetId()) {
            isNew = false;
        }
    }

    if (isNew) {
        // cout << node->m_nodeId << endl;
        node->m_values.push_back(valueID);
    }

    return isNew;


	// for (it = nodes->begin(); it != nodes->end(); it++) {
	// 	if ((*it)->m_nodeId == nodeID) {
	// 		(*it)->m_name = Manager::Get()->GetNodeProductName(valueID.GetHomeId(), nodeID);
	// 		(*it)->m_values.push_back(valueID);
	// 		return true;
	// 	}
	// }
	// return false;
}

//Removes a value from a node's values list
bool Five::removeValue(ValueID valueID) {
	uint8 nodeID{ valueID.GetNodeId() };
	list<NodeInfo*>::iterator it;
	list<ValueID>* valueIDs;
	list<ValueID>::iterator it2;

	for (it = nodes->begin(); it != nodes->end(); it++) {
		if ((*it)->m_nodeId == nodeID) {
			valueIDs = &((*it)->m_values);

			for (it2 = valueIDs->begin(); it2 != valueIDs->end(); it2++) {
				if (*it2 == valueID) {
					valueIDs->remove(valueID);
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

// If the path is right, remove the target file.
bool Five::removeFile(string path) {
	char arr[path.length()];
    strcpy(arr, path.c_str());
    remove(arr);
	return true;
}

string Five::getDriverData(uint32 homeID) {
	Driver::DriverData driverData;
	string output = "";

	Manager::Get()->GetDriverStatistics(homeID, &driverData);
	output += "   - ACK counter: ";
	output += to_string(driverData.m_ACKCnt);
	output += "   - ACK waiting: ";
	output += to_string(driverData.m_ACKWaiting);
	return output;
}

//Returns the current Date and Time
chrono::high_resolution_clock::time_point Five::getCurrentDatetime() {
    return std::chrono::high_resolution_clock::now();
}

//Converts the Date and Time to the wanted format
tm* Five::convertDateTime(chrono::high_resolution_clock::time_point datetime) {
    time_t convertTime{ std::chrono::high_resolution_clock::to_time_t(datetime) };
    return localtime(&convertTime);
}

//Returns only the time
string Five::getTime(tm* datetime) {
    return to_string(datetime->tm_hour) + ":" + to_string(datetime->tm_min) + ":" + to_string(datetime->tm_sec);
}

//Returns only the date
string Five::getDate(tm* datetime) {
    return to_string(datetime->tm_year) + "-" + to_string(datetime->tm_mon) + "-" + to_string(datetime->tm_mday);
}

//Returns the elapsed time between two date/time
double Five::difference(chrono::high_resolution_clock::time_point datetime01, chrono::high_resolution_clock::time_point datetime02) {
    chrono::duration<double> elapsed_seconds = datetime01 - datetime02;
	double rounded_elapsed = ceil(elapsed_seconds.count() * 1000) / 1000;
    return rounded_elapsed;
}

// Convert a string into an array of char.
// The output parameter must have the same length as the string.
// <output> will be filled.
void Five::stoc(string chain, char* output) {
    for (int i = 0; i < int(chain.length()); i++) {
        output[i] = chain[i];
    }
}

//Iterrates through the node list and asks the user to pick one (console command)
bool Five::nodeChoice(int* choice, list<NodeInfo*>::iterator it){
    string response;
    cout << "\n";
        for(it =Five::nodes->begin(); it !=Five::nodes->end(); it++) {
            cout << "[" << to_string((*it)->m_nodeId) << "] ";
            (*it)->m_isDead ? cout << "❌ " : cout << "✅ ";
            cout << getTime(convertDateTime((*it)->m_sync)) << ", "
                 << (*it)->m_name << "\n";
        }

        cout << "\nChoose ('q' to exit): ";
        cin >> response;
        if(response == "q"){
            *choice = -1 ;
            return true;
        }
        *choice = stoi(response);
        return true;
}
//Prints the values of a node, and eventually asks for a choice (depending on bool value)
bool Five::printValues(int* choice, list<NodeInfo*>::iterator* it, list<ValueID>::iterator it2, bool getOnly){
    string container;
    string* ptr_container = &container;
    list<string>::const_iterator sIt;
    string response;
    int counterValue(0);
    for(*it = nodes->begin(); *it != nodes->end(); (*it)++){
        if (*choice == ((**it)->m_nodeId)) {
            if(getOnly) {
                cout << "\n>>────|VALUES OF THE NODE " << to_string((**it)->m_nodeId) << "|────<<\n\n"
                     << "[<nodeId>] <label>, <value>, <readOnly>\n\n";
                for(it2 = (**it)->m_values.begin(); it2 != (**it)->m_values.end(); it2++) {
                    Manager::Get()->GetValueAsString((*it2), ptr_container);
                    cout << "[" << counterValue++ << "] "
                         << Manager::Get()->GetValueLabel(*it2) << ", "
                         << *ptr_container << ", "
                         << Manager::Get()->IsValueReadOnly(*it2)
                         << "                          " << to_string(it2->GetId()) << endl;
                }
                return true;
            } else {
                for (it2 = (**it)->m_values.begin(); it2 != (**it)->m_values.end(); it2++) {
                    if ((ValueID::ValueType_List == (*it2).GetType() || ValueID::ValueType_Button == (*it2).GetType()) && !Manager::Get()->IsValueReadOnly(*it2)) {
                        cout << "[" << ++counterValue << "] " << Manager::Get()->GetValueLabel((*it2)) << endl;
                    } else for(sIt = TYPES.begin(); sIt != TYPES.end(); ++sIt) {
                        if ((Manager::Get()->GetValueLabel((*it2)).find((*sIt)) != string::npos) && !Manager::Get()->IsValueReadOnly(*it2)) {
                            cout << "[" << ++counterValue << "] " << Manager::Get()->GetValueLabel((*it2)) << endl;
                        }
                    }
                }
                cout << "\nSelect a value ('q' to exit): ";
                cin >> response;
                if (response == "q") {
                    *choice = -1;
                    return true;
                }
                *choice = stoi(response);
                return true;
            }
            break;
        }

    }
    return true;
}

//Sets a value using specific setValue function depending on the value
bool Five::newSetValue(int* choice, list<NodeInfo*>::iterator* it, list<ValueID>::iterator it2, bool isOk){
    list<string>::const_iterator sIt;
    string container;
    string* ptr_container = &container;
    string response;
    int listchoice(0);
    int counterValue(0);

    cout << "nexset" << endl;

    for (it2 = (**it)->m_values.begin(); it2 != (**it)->m_values.end(); it2++)
    {
        cout << "test" << endl;
        if (ValueID::ValueType_Button == (*it2).GetType() && !Manager::Get()->IsValueReadOnly(*it2))
        {
            counterValue++;
            if (*choice == counterValue) {
                setButton((*it2));
                return true;
            }

        }
        if (ValueID::ValueType_List == (*it2).GetType() && !Manager::Get()->IsValueReadOnly(*it2))
        {
            counterValue++;
            if (*choice == counterValue)
            {
                Five::setList((*it2));
                return true;
            }

        }else for (sIt = TYPES.begin(); sIt != TYPES.end(); ++sIt)
        {
            if(Manager::Get()->GetValueLabel((*it2)).find((*sIt)) != string::npos && !Manager::Get()->IsValueReadOnly(*it2))
            {
                counterValue++;
                if (*choice == counterValue)
                {
                    string valLabel = Manager::Get()->GetValueLabel(*it2);
                    cout << "You chose " << valLabel << endl;
                    Manager::Get()->GetValueAsString((*it2), ptr_container);
                    cout << "Current value: " << *ptr_container << endl;
                    // cout << "Set to what ? ";
                    //cin >> response;

                    //Checking value type to choose the right method
                    if(valLabel.find("Switch") != string::npos){
                        setSwitch((*it2), true);
                        return true;
                    }else if(valLabel.find("Color") != string::npos && (*it2).GetType() == ValueID::ValueType_String)
                    {
                        setColor(*it2);
                        return true;
                    } else if(valLabel.find("Level") != string::npos && (*it2).GetType() == ValueID::ValueType_Byte)
                    {
                        cout << "Choose a value between:" << endl << "1. Very High\n" << "2. High\n" << "3. Medium\n" << "4. Low\n" << "5. Very Low\n";
                        while(!isOk){
                            cin >> response;
                            try{
                                listchoice = stoi(response);
                                isOk = true;
                            } catch(exception &err){
                                cout << "Please enter an integer" << endl;
                            }
                            return true;
                        }

                        listchoice = stoi(response);
                        switch(listchoice){
                            case 1:
                                setIntensity((*it2), IntensityScale::VERY_HIGH);
                                break;
                            case 2:
                                setIntensity((*it2), IntensityScale::HIGH);
                                break;
                            case 3:
                                setIntensity((*it2), IntensityScale::MEDIUM);
                                break;
                            case 4:
                                setIntensity((*it2), IntensityScale::LOW);
                                break;
                            case 5:
                                setIntensity((*it2), IntensityScale::VERY_LOW);
                                break;
                        }
                        return true;

                    }else if(valLabel.find("Volume") != string::npos)
                    {
                        cout << "Choose a value between:" << endl << "1. Very High\n" << "2. High\n" << "3. Medium\n" << "4. Low\n" << "5. Very Low\n";
                        cin >> response;
                        listchoice = stoi(response);
                        switch(listchoice){
                            case 1:
                                setIntensity((*it2), IntensityScale::VERY_HIGH);
                                break;
                            case 2:
                                setIntensity((*it2), IntensityScale::HIGH);
                                break;
                            case 3:
                                setIntensity((*it2), IntensityScale::MEDIUM);
                                break;
                            case 4:
                                setIntensity((*it2), IntensityScale::LOW);
                                break;
                            case 5:
                                setIntensity((*it2), IntensityScale::VERY_LOW);
                                break;
                            default:
                                break;
                        }
                        return true;
                    } else if (valLabel.find("Duration") != string::npos) {
                        setDuration((*it2));
                        return true;
                    } else if ((*it2).GetType() == ValueID::ValueType_Int) {
                        setInt(*it2);
                        return true;
                    } else if ((*it2).GetType() == ValueID::ValueType_Bool) {
                        setBool(*it2);
                        return true;
                    }

                    break;
                }
            }
        }
    }
    return true;
}

//Checks if an arg contains digits or not
bool UT_isDigit(string arg) {
    int i;

    for (i=0; i<(int)arg.size(); i++) {
        if (!isdigit(arg[i])) {
            return false;
        }
    }
    return true;
}

//Checks if the given id corresponds to an existing ValueID, and if yes places it in pointer
bool UT_isValueIdExists(string id, ValueID* ptr_valueID) {
    bool valueIdFound(false);

    for (auto it = nodes->begin(); it != nodes->end(); it++) {
        list<ValueID>::iterator it2;

        for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
            if (to_string((*it2).GetId()) == id) {
                valueIdFound = true;
                break;
            }
        }

        if (valueIdFound) {
            *ptr_valueID = *it2;
            break;
        }
    }

    return valueIdFound;
}

//Checks if the given id corresponds to an existing Node
bool UT_isNodeIdExists(string id) {
    bool nodeIdFound(false);

    for (auto it = nodes->begin(); it != nodes->end(); it++) {
        if (to_string((*it)->m_nodeId) == id) {
            nodeIdFound = true;
            break;
        }
    }
    return nodeIdFound;
}

//Builds the message to php client in a json format
string Five::buildPhpMsg(string commandName, vector<string> args) {
    Message msg = Message::InvalidCommand;
    StatusCode status = StatusCode::INVALID_badRequest;
    ValueID valueID;
    
    auto awakeTime = (getCurrentDatetime().time_since_epoch().count() - startedAt.time_since_epoch().count()) / 1000000000;

    string body = "";
    body += "\"upTime\": " + to_string(awakeTime) + ", ";
    body += "\"commandName\": \"" + commandName + "\", ";
    body += "\"args\": [";

    for (auto it = args.begin(); it != args.end(); it++) {
        if (it != args.begin()) {
            body += ", ";
        }

        body += '\"' + *it + '\"';
    }

    body += "], \"body\": { ";

    if (commandName == COMMANDS[0].name) { // setValue
        if (args.size() != 2) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else if (!UT_isDigit(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::InvalidArgument;
        } else if (!UT_isValueIdExists(args[0], &valueID)) {
            status = StatusCode::INVALID_notFound;
            msg = Message::ValueNotFoundError;
        } else{
            status = StatusCode::VALID_accepted;
            msg = Message::None;
            Manager::Get()->SetValue(valueID, args[1]);
        }
    } else if (commandName == COMMANDS[1].name) { // include
        status = StatusCode::VALID_created;
        msg = Message::None;
        Manager::Get()->AddNode(Five::homeID, false);
    } else if (commandName == COMMANDS[2].name) { // exclude
        status = StatusCode::VALID_accepted;
        msg = Message::None;
        Manager::Get()->RemoveNode(Five::homeID);
    } else if (commandName == COMMANDS[3].name) { // getNode
        if ((int)args.size() == 0) {
            status = StatusCode::VALID_ok;
            msg = Message::None;
            body += "\"nodes\": [ ";
            for (auto it = nodes->begin(); it != nodes->end(); it++) {
                if (it != nodes->begin()) {
                    body += ", ";
                }
                body += nodeToJson(getNode((*it)->m_nodeId, nodes));
            }
            body += " ], ";

        } else if ((int)args.size() != 1) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else  if (!UT_isDigit(args[0])){
            status = StatusCode::INVALID_badRequest;
            msg = Message::ValueTypeError;
        } else if(!UT_isNodeIdExists(args[0])){
            status = StatusCode::INVALID_badRequest;
            msg = Message::NodeNotFoundError;
        } else {
            status = StatusCode::VALID_ok;
            msg = Message::None;

            for(auto it = nodes->begin(); it != nodes->end(); (it)++){
                if (stoi(args[0]) == ((*it)->m_nodeId)) {
                    body += "\"node\": " + nodeToJson(getNode((*it)->m_nodeId, nodes)) + ", "; // Display node information.
                }
            }
        }
    } else if (commandName == COMMANDS[4].name) { // reset
        if ((int)args.size() != 1) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else if (UT_isDigit(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ValueTypeError;
        } else if (args[0] == "hard") {
            status = StatusCode::VALID_accepted;
            msg = Message::None;
            Manager::Get()->ResetController(Five::homeID);
        } else if(args[0] == "soft") {
            status = StatusCode::VALID_accepted;
            msg = Message::None;
            Manager::Get()->SoftReset(Five::homeID);
        } else {
            status = StatusCode::INVALID_badRequest;
            msg = Message::InvalidArgument;
        }
    } else if (commandName == COMMANDS[5].name) { // heal
        if ((int)args.size() == 0) {
            status = StatusCode::VALID_noContent;
            msg = Message::None;
            Manager::Get()->HealNetwork(homeID, true);
        } else if ((int)args.size() != 1) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else if (!UT_isDigit(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ValueTypeError;
        } else if (!UT_isNodeIdExists(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::NodeNotFoundError;
        } else {
            status = StatusCode::VALID_noContent;
            msg = Message::None;
            for(auto it = nodes->begin(); it != nodes->end(); it++){
                if (stoi(args[0]) == (*it)->m_nodeId) {
                    Manager::Get()->HealNetworkNode((*it)->m_homeId, (*it)->m_nodeId, true);
                }
            }
        }
    } else if (commandName == COMMANDS[6].name) { // isFailed
        bool isFailed = false;

        if ((int)args.size() != 1) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else if (!UT_isDigit(args[0])){
            status = StatusCode::INVALID_badRequest;
            msg = Message::ValueTypeError;
        } else if(!UT_isNodeIdExists(args[0])){
            status = StatusCode::INVALID_badRequest;
            msg = Message::NodeNotFoundError;
        } else {
            status = StatusCode::VALID_noContent;
            msg = Message::None;

            for (auto it = nodes->begin(); it != nodes->end(); it++) {
                if (stoi(args[0]) == (*it)->m_nodeId) {
                    isFailed = Manager::Get()->IsNodeFailed(Five::homeID, (*it)->m_nodeId);
                    break;
                }
            }

            body += "\"isFailed\": " + to_string(isFailed) + ", ";
        }
    } else if (commandName == COMMANDS[7].name) { // ping
        string message;
        int countTrue = 0;
        int countFalse = 0;

        if ((int)args.size() != 2) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        }else for(int i = 0; i < 2; i++) {
            if (!UT_isDigit(args[i])){
                status = StatusCode::INVALID_badRequest;
                msg = Message::ValueTypeError;
            }
        } 
        if (!UT_isNodeIdExists(args[0])) {
            status = StatusCode::INVALID_badRequest;
            msg = Message::NodeNotFoundError;
        } else {
            status = StatusCode::VALID_noContent;
            msg = Message::None;
            for (auto it = nodes->begin(); it != nodes->end(); it++) {
                if (args[0] == to_string((*it)->m_nodeId)) {
                    for (auto it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++) {
                        if (Manager::Get()->GetValueLabel(*it2) == "Library Version") {
                            int counter{stoi(args[1])};
                            while (counter --> 0) {
                                Manager::Get()->RefreshValue(*it2);
                                // msg = "ask node " + to_string((*it)->m_nodeId) + " ... " + to_string(Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)) + "\n";
                                // sendMsg(LOCAL_ADDRESS, PHP_PORT,  msg);
                                if(Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)){
                                    countTrue += 1;
                                } else{
                                    countFalse += 1;
                                }
                                this_thread::sleep_for(chrono::milliseconds(500));
                            }

                        }
                    }
                }
            }
        }
        body = "\"Ping sent\": " + args[1] + ", \"received\": " + to_string(countTrue) + ", \"failed\": " + to_string(countFalse) + ", ";
    } else if (commandName == COMMANDS[8].name) { // help
        status = StatusCode::VALID_ok;
        msg = Message::None;
        int cmdCounter = sizeof(COMMANDS)/sizeof(COMMANDS[0]);

        body += "\"commands\": [ ";
        for (int i = 0; i < cmdCounter; i++) {
            if (i != 0) {
                body += ", ";
            }
            body += "{ \"name\": \"" + COMMANDS[i].name + "\", ";
            body += "\"args\": [ ";
            for (auto it = COMMANDS[i].arguments.begin(); it != COMMANDS[i].arguments.end(); it++) {
                if (it != COMMANDS[i].arguments.begin()) {
                    body += ", ";
                }
                body += '\"' + *it + '\"';
            }
            body += " ], \"description\": \"" + COMMANDS[i].description + "\" }";
        }
        body += " ], ";
    } else if (commandName == COMMANDS[9].name) { //broadcast
        string message;
        int countTrue = 0;
        int countFalse = 0;
        bool pinged(false);

        if((int)args.size() != 0){
            status = StatusCode::INVALID_badRequest;
            msg = Message::ArgumentError;
        } else {
            for(auto it = nodes->begin(); it != nodes->end(); ++it){
                for(auto it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++){
                    if (Manager::Get()->GetValueLabel(*it2) == "Library Version") {
                        int counter{ 60 };
                        while (counter --> 0) {
                            Manager::Get()->RefreshValue(*it2);
                            this_thread::sleep_for(chrono::milliseconds(500));
                            message = "ask node " + to_string((*it)->m_nodeId) + " ... " + to_string(Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)) + "\n";
                            sendMsg(LOCAL_ADDRESS, PHP_PORT, message);
                            if(Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId)){
                                counter = 0;
                                pinged = true;
                            }
                        }
                    }
                    if(pinged){
                        countTrue += 1;
                    } else{
                        countFalse += 1;
                    }
                }
            }
            body += "\"successful nodes\": " + to_string(countTrue) + " \"failed nodes\": " + to_string(countFalse) + ", ";

        }
    } else if (commandName == COMMANDS[10].name) { // Restart.
        status = StatusCode::VALID_ok;
        msg = Message::None;
        Manager::Get()->RemoveDriver(DRIVER_PATH);
        cout << system("sudo systemctl restart minozw.service") << endl;
    } else if (commandName == COMMANDS[11].name) { // Reset.
        status = StatusCode::VALID_ok;
        msg = Message::None;
        Manager::Get()->RemoveDriver(DRIVER_PATH);
        cout << system("./cpp/examples/bash/reset_key.sh") << endl;
    }

    body += "\"status\": " + to_string(status);
    body += ", \"message\": \"" + messages[msg] + "\" } }";

    int body_length = body.length();
    body = "{ \"messageLength\": " + to_string(body_length + to_string(body_length).length()) + ", " + body;

    return body;
}

//Receives and translates the messages received from PHP client
string Five::receiveMsg(sockaddr_in address, int server_fd) {
    vector<string> args;
    char *ptr;
    ValueID valueID;

    string output("");
    string myFunc("");
    int counter(0);
    int new_socket(0);
    int valread(0);
    char buffer[1024] = {0};
    int addrlen = sizeof(address);

    for (int i = 0; i < 1024; i++) {
        buffer[i] = 0;
    }

    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    valread = read(new_socket, buffer, 1024);
    cout << valread << endl;

    output += "[" + getTime(convertDateTime(chrono::high_resolution_clock::now()));
    output += "] ";
    output += inet_ntoa(address.sin_addr);
    output += ":" + to_string(ntohs(address.sin_port));
    output += ", message(";
    output += to_string(valread);
    output += "): \"";
    output += buffer;
    output += "\" --> status: ";

    ptr = strtok(buffer, ",");

    while (ptr != NULL) {
        if (counter++ == 0) {
             for (unsigned long i = 0; i < sizeof(ptr); i++) {
                 if(ptr[i] != '\0'){
                     myFunc += ptr[i];
                 }
             }
        } else {
            args.push_back(ptr);
        }

        ptr = strtok(NULL, ",");
    }

    string msg = buildPhpMsg(myFunc, args);

    char fMessage[msg.length() + 1];
    strcpy(fMessage, msg.c_str());

    for (int i = 0; i < (int)msg.length(); i++) {
        fMessage[i] = msg[i];
    }

    send(new_socket, fMessage, strlen(fMessage), 0);
    return output;
}

void Five::server(int port) {
	int server_fd;
	struct sockaddr_in address;
	int opt = 1;

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	bind(server_fd, (struct sockaddr*)&address, sizeof(address));
	listen(server_fd, 3);
    cout << "Listening on port " << port << endl;

    while (true) {
        cout << receiveMsg(address, server_fd) << endl;
    }
}

int Five::sendMsg(const char* address, const int port, string message) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(address);

    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return EXIT_FAILURE;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed \n");
        return EXIT_FAILURE;
    }

    // cout << "-> MSG: " << to_string(message.length()) << endl;
    // cout << message << endl;

    char fMessage[message.length() + 1];
    strcpy(fMessage, message.c_str());

    for (int i = 0; i < (int)message.length(); i++) {
        fMessage[i] = message[i];
    }

    send(sock, fMessage, strlen(fMessage), 0);

    return EXIT_SUCCESS;
}

//Converts a char array into a string
string convertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

//Converts the information of a node into json format suitable to send
string Five::nodeToJson(NodeInfo* node) {
    string msg = "";
    msg += "{ \"nodeId\": " + to_string(node->m_nodeId);
    msg += ", \"productName\": \"" + node->m_name;
    msg += "\", \"nodeDead\": " + to_string(node->m_isDead);
    msg += ", \"lastUpdate\": \"";

    list<ValueID> values = node->m_values;
    string date = getDate(convertDateTime(node->m_sync));
    string time = getTime(convertDateTime(node->m_sync));

    msg += date + " " + time;
    msg += "\", \"valueIds\": [ ";

    for (auto it=values.begin(); it!=values.end(); it++) {
        if (it != values.begin()) {
            msg += ", ";
        }

        msg += valueIdToJson(*it);
    }

    if(node->m_nodeId > 1){
        msg += ", \"nodeNeighbors\": \"";
        for(int i = 0; i < 29; ++i){
            if (i != 0) {
                msg += ", ";
            }
            msg += to_string(*(node->m_neighbors)[i]);
        }
    }

    msg += " ] }";
    return msg;
}

//Converts the information of a ValueID into json format suitable to send
string Five::valueIdToJson(ValueID valueId) {
    vector<string> prop;
    string msg = "";
    msg += "{ \"id\": \"";
    msg += to_string(valueId.GetId());
    msg += "\", \"readonly\": ";
    msg += to_string(Manager::Get()->IsValueReadOnly(valueId));
    msg += ", \"label\": \"";
    msg += Manager::Get()->GetValueLabel(valueId);
    msg += "\", \"value\": ";

    ValueID::ValueType valType = valueId.GetType();
    bool isNumeric(false);
    string value;

    Manager::Get()->GetValueAsString(valueId, &value);

    for (int i=0; i<(int)(sizeof(NUMERIC_TYPES)/sizeof(int)); i++) {
        if (NUMERIC_TYPES[i] == valType) {
            isNumeric = true;
            break;
        }
    }

    if (valType == ValueID::ValueType_Bool) {
        value[0] = tolower(value[0]);
    }

    if (isNumeric) {
        msg += value;
    } else {
        if (value == "False" || value == "True") {
            value[0] = tolower(value[0]);
            msg += value;
        } else {
            msg += "\"";
            msg += value;
            msg += "\"";
        }
    }

    if(valType == ValueID::ValueType_List){
        msg += ", \"options\": [ \"";
        Manager::Get()->GetValueListItems(valueId, &prop);
        for(auto it = prop.begin(); it != prop.end(); it++){
            if(it == prop.begin()){
                msg += (*it) + "\"";
            }else{
                msg += ", \"" + (*it) + "\"";
            }
        }
        msg += " ]";
    }

    msg += " }";

    return msg;
}

string Five::buildNotifMsg(Notification const *notification) {
    string msg = "";
    msg += "\"notificationType\": \"";
    msg += NOTIFICATIONS[(int)notification->GetType()];
    // msg += "\", \"object\": { \"nodeId\": ";
    msg += "\", \"object\": ";
    msg += nodeToJson(getNode(notification->GetNodeId(), nodes));

    msg += " }";

    msg = ", " + msg;
    string header = "{ \"msgLength\": ";
    int msgLength = msg.length() + header.length();

    msg = to_string(msgLength + to_string(msgLength).length()) + msg;
    msg = header + msg;
    return msg;
}

void Five::statusObserver(list<NodeInfo*> *nodes) {
    while(true) {
		fstream file;
		string line;
		list<NodeInfo*>::iterator it;
		list<ValueID>::iterator it2;
		int32 wakeUpInterval;

		// Get the current counter from epoch.
		int64 now = chrono::high_resolution_clock::now().time_since_epoch().count();


		Driver::ControllerState newState( Driver::ControllerState::ControllerState_Error );

		if (homeID != 0)
			newState = Manager::Get()->GetDriverState(homeID);

		if (driverState != newState) {
			cout << "State: " << STATES[newState] << "\n";
			driverState = newState;
		}

		for (it = nodes->begin(); it != nodes->end(); it++) {
			list<ValueID> valueIDs = (*it)->m_values;

			for (it2 = valueIDs.begin(); it2 != valueIDs.end(); it2++) {
				string label = Manager::Get()->GetValueLabel(*it2);

				if (label.find("Wake-up Interval") != string::npos) { // The object has a sleep interval.
					Manager::Get()->GetValueAsInt(*it2, &wakeUpInterval); // Stores the object wakeUpInteral value.

					// Check if (now - lastUpdate) is greater than the wakeUpInterval
					// or if the object is not synchronized yet.
					if (((now - (*it)->m_sync.time_since_epoch().count()) > (wakeUpInterval * pow(10, 9))) || convertDateTime((*it)->m_sync)->tm_hour == 1) {
						(*it)->m_isDead = true;

						// file.open(FAILED_NODE_PATH, ios::in);
						// while(getline(file, line)){
						// 	if(line.find("Label: " + (*it)->m_name) != string::npos){
						// 		isIn = 1;
						// 	}
						// }
						// file.close();
						// if(!isIn){
						// 	file.open(FAILED_NODE_PATH, ios::app);
						// 	file << "Label: " << (*it)->m_name << " Id: " << unsigned((*it)->m_nodeId) << " Type: " << (*it)->m_nodeType << endl;
						// 	file.close();
						// }
					}else {
						(*it)->m_isDead = false;

						// file.open(FAILED_NODE_PATH, ios::in);
						// fstream temp;
						// temp.open("temp.txt", ios::app);
						// while(getline(file, line)){
						// 	cout << line << endl;
						// 	string s = "Label: " + (*it)->m_name;
						// 	cout << s << endl;
						// 	if(line.find(s) == string::npos){
						// 		temp << line;
						// 	}
						// }
						// temp.close();
						// file.close();

						// const char *p = FAILED_NODE_PATH.c_str();
						// remove(p);
						// rename("temp.txt", p);
					}
					break;
				}
			}
		}

		this_thread::sleep_for(chrono::milliseconds(Five::OBSERVER_PERIOD));
	}
}

void Five::onNotification(Notification const* notification, void* context) {
	ofstream myfile;
	ValueID valueID{ notification->GetValueID() };
	string valueLabel;
	uint8 cc_id{ valueID.GetCommandClassId() };
	string cc_name{ Manager::Get()->GetCommandClassName(cc_id) };
	string path = NODE_LOG_PATH + "node_" + to_string(notification->GetNodeId()) + ".log";
	string container;
	string *ptr_container = &container;
	string notifType{ "" };
	string log{ "" };

	if (notification->GetType() == Notification::Type_ValueChanged) {
		string msg = buildNotifMsg(notification);

		// thread ttemp(sendMsg, "0.0.0.0", PHP_PORT, msg);
		// ttemp.join();
	}

	Node::NodeData nodeData;
	Node::NodeData* ptr_nodeData = &nodeData;

	pthread_mutex_lock(&g_criticalSection); // Lock the critical section
	
	if (Five::homeID == 0) {
		Five::homeID = notification->GetHomeId();
	}

	list<NodeInfo*>::iterator it;

	switch (notification->GetType()) {
		case Notification::Type_ValueAdded:
			notifType = "VALUE ADDED";
			log += "[VALUE_ADDED]	                  node " + to_string(notification->GetNodeId()) + ", value " + to_string(valueID.GetId()) + '\n';

			for (it = nodes->begin(); it != nodes->end(); it++) {
				if (notification->GetNodeId() == (*it)->m_nodeId) {
					(*it)->m_name = Manager::Get()->GetNodeProductName(homeID, notification->GetNodeId());
				}
			}

			addValue(valueID, getNode(notification->GetNodeId(), Five::nodes));
			break;
		case Notification::Type_ValueRemoved:
			log += "[VALUE_REMOVED]                   node "
			    + to_string(notification->GetNodeId()) + " value "
				+ to_string(valueID.GetId()) + '\n';

			notifType = "VALUE REMOVED";
			removeValue(valueID);
			break;
		case Notification::Type_ValueChanged:
			notifType = "VALUE CHANGED";

			Manager::Get()->GetValueAsString(valueID, ptr_container);
			Manager::Get()->GetNodeStatistics(valueID.GetHomeId(), valueID.GetNodeId(), ptr_nodeData);

			log += "[VALUE_CHANGED]                   node "
			    + to_string(valueID.GetNodeId()) + ", "
				+ Manager::Get()->GetValueLabel(valueID) + ": "
				+ *ptr_container + '\n';

			valueID = notification->GetValueID();
			break;
		case Notification::Type_ValueRefreshed:
			notifType = "VALUE REFRESHED";
			Manager::Get()->GetValueAsString(valueID, ptr_container);
			log += "[VALUE_REFRESHED]                 node " + to_string(valueID.GetNodeId()) + ", "
				+ Manager::Get()->GetValueLabel(valueID) + ": " + *ptr_container + "\n";
			break;
		case Notification::Type_Group:
			notifType = "GROUP";
			break;
		case Notification::Type_NodeNew:
			notifType = "NODE NEW";
			break;
		case Notification::Type_NodeAdded:
			log += "[NODE_ADDED]                      node " + to_string(notification->GetNodeId()) + '\n';
			notifType = "NODE ADDED";

			pushNode(notification, Five::nodes);
			break;
		case Notification::Type_NodeRemoved:
			log += "[NODE_REMOVED]                    node " + to_string(notification->GetNodeId()) + '\n';
			notifType = "NODE REMOVED";

			removeFile(path);
			removeNode(notification, Five::nodes);
			break;
		case Notification::Type_NodeProtocolInfo:
			notifType = "NODE PROTOCOL INFO";
			break;
		case Notification::Type_NodeNaming:
			log += "[NODE_NAMING]                     node " + to_string(valueID.GetNodeId()) + '\n';
			notifType = "NODE NAMING";
			break;
		case Notification::Type_NodeEvent:
			notifType = "NODE EVENT";
			break;
		case Notification::Type_PollingDisabled:
			notifType = "POLLING DISABLED";
			break;
		case Notification::Type_PollingEnabled:
			notifType = "POLLING ENABLED";
			break;
		case Notification::Type_SceneEvent:
			notifType = "SCENE EVENT";
			break;
		case Notification::Type_CreateButton:
			notifType = "CREATE BUTTON";
			break;
		case Notification::Type_DeleteButton:
			notifType = "DELETE BUTTON";
			break;
		case Notification::Type_ButtonOn:
			notifType = "BUTTON ON";
			break;
		case Notification::Type_ButtonOff:
			notifType = "BUTTON OFF";
			break;
		case Notification::Type_DriverReady:
			log += "[DRIVER_READY]                    driver READY\n" + getDriverData(notification->GetHomeId()) + '\n';
			notifType = "DRIVER READY";
			break;
		case Notification::Type_DriverFailed:
			notifType = "DRIVER FAILED";
			break;
		case Notification::Type_DriverReset:
			notifType = "DRIVER RESET";
			break;
		case Notification::Type_EssentialNodeQueriesComplete:
			log += "[ESSENTIAL_NODE_QUERIES_COMPLETE] node " + to_string(notification->GetNodeId()) + ", queries COMPLETE" + '\n';
			notifType = "ESSENTIAL NODE QUERIES COMPLETE";
			break;
		case Notification::Type_NodeQueriesComplete:
			log += "[NODE_QUERIES_COMPLETE]           node " + to_string(valueID.GetNodeId()) + '\n';
			notifType = "NODE QUERIES COMPLETE";
			break;
		case Notification::Type_AwakeNodesQueried:
			notifType = "AWAKE NODES QUERIED";
			break;
		case Notification::Type_AllNodesQueriedSomeDead:
			log += "\n🚨 [ALL_NODES_QUERIED_SOME_DEAD]  node " + to_string(valueID.GetNodeId()) + '\n'
			     + "   - total: " + to_string(aliveNodeSum(nodes) + deadNodeSum(nodes)) + '\n'
			     + "   - alive: " + to_string(aliveNodeSum(nodes)) + '\n'
			     + "   - dead : " + to_string(deadNodeSum(nodes)) + '\n'
			     + "   - start: " + getTime(convertDateTime(startedAt))
			     + "   - elapse: " + to_string(difference(getCurrentDatetime(), startedAt)) + "s\n" + '\n';
			notifType = "ALL NODES QUERIED SOME DEAD";
			break;
		case Notification::Type_AllNodesQueried:
			log += "\n✅ [ALL_NODES_QUERIED]            node " + to_string(valueID.GetNodeId()) + '\n'
			     + "   - total: " + to_string(aliveNodeSum(nodes) + deadNodeSum(nodes)) + '\n'
			     + "   - alive: " + to_string(aliveNodeSum(nodes)) + '\n'
			     + "   - dead : " + to_string(deadNodeSum(nodes)) + '\n'
			     + "   - start: " + getTime(convertDateTime(startedAt))
			     + "   - elapse: " + to_string(difference(getCurrentDatetime(), startedAt)) + "s\n" + '\n';
			notifType = "ALL NODES QUERIED";
			break;
		case Notification::Type_Notification:
			notifType = "NOTIFICATION";
			break;
		case Notification::Type_DriverRemoved:
			notifType = "DRIVER REMOVED";
			break;
		case Notification::Type_ControllerCommand:
			notifType = "CONTROLLER COMMAND";
			break;
		case Notification::Type_NodeReset:
			notifType = "NODE RESET";
			break;
		case Notification::Type_UserAlerts:
			notifType = "USER ALERTS";
			break;
		case Notification::Type_ManufacturerSpecificDBReady:
			// The valueID is empty, you can't use it here.
			log += "[MANUFACTURER_SPECIFIC_DB_READY]  manufacturer database READY" + '\n';
			notifType = "MANUFACTURER SPECIFIC DB READY";
			break;
		default:
			break;
	}

	if (notifType == "") {
		notifType = to_string(notification->GetType());
	}

	switch (Five::LEVEL)
	{
	case Five::logLevel::DEBUG:
		cout << log;
		break;
	default:
		break;
	}

	// cout << ">> " << notifType << endl;
	// cout << log;

	if (containsType(notification->GetType(), Five::AliveNotification) || notification->GetNodeId() == 1) {
		// if ((containsType(notification->GetType(), Five::AliveNotification) || (nodes->size() == 1 && notification->GetType() == Notification::Type_AllNodesQueried)) && g_menuLocked) {
		// 	thread t1(menu);
		// 	t1.detach();
		// 	g_menuLocked = false;
		// }

		if (containsNodeID(notification->GetNodeId(), (*Five::nodes))) {
			NodeInfo* n = getNode(notification->GetNodeId(), Five::nodes);
			if (n->m_isDead) {
				n->m_isDead = false;
				if (LEVEL != logLevel::NONE && n->m_nodeId != 1) {
					cout << "\n\n⭐ [NEW_NODE_APPEARS]             node " << to_string(valueID.GetNodeId()) << endl;
					cout << "   - total: " << aliveNodeSum(nodes) + deadNodeSum(nodes) << endl;
					cout << "   - alive: " << aliveNodeSum(nodes) << endl;
					cout << "   - dead : " << deadNodeSum(nodes) << "\n\n" << endl;
				}
			}
			n->m_sync = chrono::high_resolution_clock::now();
		}
	}

	vector<Notification::NotificationType> _notifs = {
		Notification::Type_ValueAdded,
		Notification::Type_AllNodesQueried,
		Notification::Type_NodeAdded,
		Notification::Type_ValueChanged,
		Notification::Type_ValueRefreshed,
		Notification::Type_NodeQueriesComplete,
	};

	switch (LEVEL)
	{
	case logLevel::DEBUG:
		if (notification->GetType() != Notification::Type_NodeRemoved) {
			myfile.open(path, ios::app);

			myfile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] "
				<< notifType << ", " << cc_name << " --> "
				<< to_string(valueID.GetIndex()) << "(" << valueLabel << ")\n";

			myfile.close();
		}
		break;
	case logLevel::INFO:
		if (containsType(notification->GetType(), _notifs)) {
			myfile.open(path, ios::app);

			myfile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] "
				<< notifType << ", " << cc_name << " --> "
				<< to_string(valueID.GetIndex()) << "(" << valueLabel << ")\n";

			myfile.close();
		}

		break;
	case logLevel::WARNING:
		if (notification->GetType() == Notification::Type_DriverFailed) {
			myfile.open(path, ios::app);

			myfile << "[" << getDate(convertDateTime(getCurrentDatetime())) << ", " << getTime(convertDateTime(getCurrentDatetime())) << "] "
				<< notifType << ", " << cc_name << " --> "
				<< to_string(valueID.GetIndex()) << "(" << valueLabel << ")\n";

			myfile.close();
		}
		
		break;
	default:
		break;
	}

	pthread_mutex_unlock(&g_criticalSection); // Unlock the critical section.
}

void Five::watchState(uint32 homeID, int loopTimeOut) {
	Driver::ControllerState referenceState{ Manager::Get()->GetDriverState(homeID) };
	Driver::ControllerState currentState{ Manager::Get()->GetDriverState(homeID) };

	while (loopTimeOut --> 0) {
		bool outOfRange{ referenceState <= 0 };

		if (!outOfRange)
			outOfRange = referenceState > STATES->size();

		if (!outOfRange)
			outOfRange = currentState <= 0;

		if (!outOfRange)
			outOfRange = currentState > STATES->size();

		cout << "State: " << Five::STATES[currentState] << "\n";

		switch (currentState) {
			case Driver::ControllerState::ControllerState_Completed:
				cout << "Final state: " << STATES[currentState] << "\n";
				return;
			// case Driver::ControllerState::ControllerState_Error:
			// 	cout << "Final state: " << STATES[currentState] << "\n";
			// 	return;
			default:
				if (referenceState != currentState) {
					referenceState = currentState;
				}

				break;
		}

		currentState = Manager::Get()->GetDriverState(homeID);

		this_thread::sleep_for(chrono::milliseconds(Five::STATE_PERIOD));
	}
	cout << "TimeOut\n";
}

void menu() {
    uint8 *bitmap[29];
	bool menuRun(1);

	while (menuRun)
	{
		string response;
		bool isOk = false;
		list<string>::iterator sIt;
		int choice{0};
		int lock(0);
		int stateInt(0);
		int counterNode{0};
		int counterValue{0};
		list<NodeInfo *>::iterator it;
		list<ValueID>::iterator it2;
		string container;
		string *ptr_container = &container;
		string fileName{""};

		// while (x --> 0) {
		// 	this_thread::sleep_for(chrono::seconds(1));
		// }

		cout << "\n>>──────|MENU|──────<<\n\n"
			 << "[-1] Dev\n"
			 << "[1] Add node\n"
			 << "[2] Remove node\n"
			 << "[3] Get value\n"
			 << "[4] Set value (old)\n"
			 << "[5] Reset Key\n"
			 << "[7] Heal\n"
			 << "[8] Set value (new)\n"
			 << "[9] Network\n"
			 << "[10] Is Node Failed\n"
			 << "\nChoose: ";

		cin >> response;

		try
		{
			choice = stoi(response);
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what() << '\n';
		}

		int milliCounter{0};

		switch (choice)
		{
		case 11:
			nodeChoice(&choice, it);

			Manager::Get()->RequestAllConfigParams(homeID, choice);
			break;
		case 10:
			cout << "Choose what node to check: " << endl;

			// Printing node names and receiving user's choice
			for (it = Five::nodes->begin(); it != Five::nodes->end(); it++)
			{
				counterNode++;
				cout << counterNode << ". " << (*it)->m_name << endl;
			}

			cin >> response;
			choice = stoi(response);
			cout << Manager::Get()->IsNodeFailed(homeID, (unsigned int)choice) << endl;
			break;

		case -1:
			cout << "\n>>────|DEV|────<<\n\n";
			for (it = nodes->begin(); it != nodes->end(); it++)
			{
				cout << "[" << to_string((*it)->m_nodeId) << "] "
					 << (*it)->m_name << "\n";
			}

			cout << "\nSelect a node ('q' to exit): ";
			cin >> response;
			for (it = nodes->begin(); it != nodes->end(); it++)
			{
				if (response == to_string((*it)->m_nodeId))
				{
					while (true)
					{
						cout << "Is awake: ";
						if (Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId))
						{
							cout << "1\n";
							break;
						}
						cout << "elapsed: " << to_string(milliCounter) << "ms, 0\n";
						this_thread::sleep_for(chrono::milliseconds(100));
						milliCounter += 100;
					}
					break;
				}
			}
			break;
		case 9:
			cout << "\n>>───|NETWORK|───<<\n\n"
				 << "[1] Ping\n"
				 << "[2] Broadcast\n"
				 << "[3] Neighbors\n"
				 << "[4] Polls\n"
				 << "[5] Map\n"
				 << "\nChoose ('q' to exit): ";

			cin >> response;

			if (response == "1")
			{
				cout << "\n>>───|PING|───<<\n\n";

				for (it = nodes->begin(); it != nodes->end(); it++)
				{
					cout << "       [ " << to_string((*it)->m_nodeId) << " ]\n";
				}
				cout << "\nChoose ('q' to exit): ";
				cin >> response;

				for (it = nodes->begin(); it != nodes->end(); it++)
				{
					cout << response << " " << to_string((*it)->m_nodeId) << '\n';
					if (response == to_string((*it)->m_nodeId))
					{
						for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
						{
							if (Manager::Get()->GetValueLabel(*it2) == "Library Version")
							{
								cout << "Ping sent...\n";
								int counter{60};
								while (counter-- > 0)
								{
									Manager::Get()->RefreshValue(*it2);

									this_thread::sleep_for(chrono::milliseconds(500));
									cout << "ask..." << Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId) << "\n";
								}
								cout << "Done\n";
							}
						}
					}
				}
			}
			else if (response == "5")
			{
				// cout << "Hello\n";
				// cout << nodes->size() <<  "Hello\n";
				// cout << lastNode << "Hello\n";
				// it = nodes->end();
				// uint8 lastNode = (*it)->m_nodeId;

				int nCounter(nodes->size());
				int matrix[nCounter][nCounter];

				for (int i = 0; i < nCounter; i++)
				{
					for (int j = 0; j < nCounter; j++)
					{
						matrix[i][j] = 0;
					}
				}

				// cout << "Setup matrix:\n";
				for (int i = 0; i < nCounter; i++)
				{
					for (int j = 0; j < nCounter; j++)
					{
						// cout << matrix[i][j] << " ";
					}
					// cout << "\n";
				}

				// int nodeIds[nCounter];

				for (int i = 0; i < nCounter; i++)
				{
					it = nodes->begin();
					advance(it, i);
					// nodeIds[i] = (*it)->m_nodeId;
				}

				// cout << "Length: " << to_string(nCounter) << "\n";

				for (int i = 0; i < nCounter; i++)
				{ // Nodes loop.
					it = nodes->begin();
					advance(it, i);
					Manager::Get()->GetNodeNeighbors(homeID, (*it)->m_nodeId, bitmap);
					// cout << "Node " << to_string((*it)->m_nodeId) << "\n";

					if (!(*it)->m_isDead && (*it)->m_nodeId != 1)
					{ // Check if the node is not the controller and can returns its neighbors.
						cout << "\n";
						for (int j = 0; j < 29; j++)
						{ // Neigbors loop.
							cout << to_string((*bitmap)[j]) << " ";
						}
						cout << "\n";

						// cout << "\n";

						// cout << "Neigbors: ";
						for (int j = 0; j < 29; j++)
						{ // Neigbors loop.
							for (int k = 0; k < (int)nodes->size(); k++)
							{
								auto jt = nodes->begin();
								advance(jt, k);
								if ((*jt)->m_nodeId == (*bitmap)[j])
								{
									// cout << to_string((*jt)->m_nodeId) << " ";
									matrix[i][k] = 1;
									break;
								}
							}
							// for (auto jt = nodes->begin(); jt != nodes->end(); jt++) {
							// 	if ((*jt)->m_nodeId == (*bitmap)[j]) {
							// 		cout << to_string((*jt)->m_nodeId) << " ";
							// 		matrix[i][]
							// 		break;
							// 	}
							// }
						}
						cout << "\n";

						// cout << to_string((*it)->m_nodeId) << "\n";

						// for (int j = 0; j < 29; j++) { // Neigbors loop.
						// 	bool eof(true);

						// 	for (int k = 0; k < nCounter; k++) {
						// 		cout << "nodeId " << to_string(nodeIds[k]) << ": " << to_string((*bitmap)[j]) << "\n";
						// 		if ((*bitmap)[j] == nodeIds[k]) { // Check if the current value is a neigbor.
						// 			eof = false;
						// 			break;
						// 		}
						// 	}

						// 	if (eof) {
						// 		cout << "eof\n";
						// 		break;
						// 	} else {
						// 		// cout << (*it)->m_nodeId << ": " << (*bitmap)[j] << "\n";
						// 		matrix[i][j] = (*bitmap)[j];
						// 	}
						// }
					}
				}

				// for (int i = 0; i < nCounter; i++) {
				// 	for (int j = 0; j < nCounter; j++) {
				// 		cout << matrix[i][j] << " ";
				// 	}
				// 	cout << "\n";
				// }

				// for (it = nodes->begin(); it != nodes->end(); it++) {
				// 	if (!(*it)->m_isDead && (*it)->m_nodeId != 1) {
				// 		cout << Manager::Get()->GetNodeNeighbors(homeID, (*it)->m_nodeId, bitmap) << "\n";

				// 		for (int i = 0; i < 29; i++) {
				// 			// if ((*bitmap)[i] > lastNode || (*bitmap)[i] == 0) {
				// 			// 	cout << "Endf\n";
				// 			// }
				// 			// cout << to_string((*bitmap)[i]) << " ";

				// 			for (auto iter = nodes->begin(); iter != nodes->end(); iter++) {
				// 				if ((*iter)->m_nodeId == (*bitmap)[i]) {
				// 					matrix[(*iter)->m_nodeId][(*iter)->m_nodeId]++;
				// 					break;
				// 				}
				// 			}
				// 		}
				// 	}
				// }

				for (int i = 0; i < nCounter; i++)
				{
					for (int j = 0; j < nCounter; j++)
					{
						cout << matrix[i][j] << " ";
					}
					cout << "\n";
				}
			}
			else if (response == "2")
			{
				cout << "\n>>─────|BROADCAST|─────<<\n\n";

				for (it = nodes->begin(); it != nodes->end(); ++it)
				{
					for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
					{
						if (Manager::Get()->GetValueLabel(*it2) == "Library Version")
						{
							cout << "Ping sent...\n";
							int counter{60};
							while (counter-- > 0)
							{
								Manager::Get()->RefreshValue(*it2);

								this_thread::sleep_for(chrono::milliseconds(500));
								if (Manager::Get()->IsNodeAwake(homeID, (*it)->m_nodeId))
								{
									cout << (*it)->m_name << ": OK" << endl;
									break;
								}
								else
								{
									cout << (*it)->m_name << ": not OK" << endl;
								}
							}
						}
					}
				}
			}
			else if (response == "3")
			{
				cout << "\n>>─────|NEIGHBORS|─────<<\n\n";

				for (it = nodes->begin(); it != nodes->end(); it++)
				{
					if ((*it)->m_nodeId != 1 && !(*it)->m_isDead)
					{ // The driver doesn't have neighbors property
						cout << "[" << to_string((*it)->m_nodeId) << "] " << (*it)->m_name << "\n";
					}
				}
				cout << "\nSelect a node ('q' to exit): ";
				cin >> response;
				cout << "\nNeighbor chain: ";
				for (it = nodes->begin(); it != nodes->end(); it++)
				{
					if (to_string((*it)->m_nodeId) == response)
					{
						Manager::Get()->GetNodeNeighbors(homeID, (*it)->m_nodeId, bitmap);
						for (int i = 0; i < 29; i++)
						{
							// for (j = 0; j < 8; j++) {
							// 	cout << (bitset<8>((*bitmap)[i]))[i] << "  ";
							// }
							cout << to_string((*bitmap)[i]);
						}
						cout << "\n\n"
							 << "[1] Synchronize neighbors\n"
							 << "[2] Request neighbor update\n "
							 << "[3] Heal network\n"
							 << "\nChoose ('q' to exit): ";
						cin >> response;

						if (response == "1")
						{
							Manager::Get()->SyncronizeNodeNeighbors(homeID, (*it)->m_nodeId);
							cout << "Done.\n";
						}
						else if (response == "2")
						{
							thread ttemp(watchState, homeID, LOOP_TIMEOUT);
							ttemp.detach();

							Manager::Get()->RequestNodeNeighborUpdate(homeID, (*it)->m_nodeId)
								? cout << "Update request sent\n"
								: cout << "Update request failed\n";
						}
						else if (response == "3")
						{
							Manager::Get()->HealNetwork(homeID, true);
						}
						break;
					}
				}
			}
			else if (response == "4")
			{
				cout << "\n>>─────|POLLS|─────<<\n\n"
					 << "/!\\ If you set the poll intensity, you must restart the ZWave key to add modification.\n\n"
					 << "[1] Set intensity\n"
					 << "[2] Set interval for all devices (PLEASE BE CAREFUL)\n\n"
					 << "Choose ('q' to exit): ";

				cin >> response;
				cout << "\n";

				if (response == "1")
				{
					for (it = nodes->begin(); it != nodes->end(); it++)
					{
						cout << "[" << to_string((*it)->m_nodeId) << "] " << (*it)->m_name << "\n";
					}
					cout << "\nSelect a node ('q' to exit): ";
					cin >> response;
					cout << '\n';

					for (it = nodes->begin(); it != nodes->end(); it++)
					{
						if (response == to_string((*it)->m_nodeId))
						{
							for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
							{
								cout << "[" << to_string(counterValue++) << "] " << Manager::Get()->GetValueLabel(*it2)
									 << ", " << to_string(Manager::Get()->GetPollIntensity(*it2)) << "\n";
							}

							cout << "\nSelect a valueID (press 'q' to exit): ";
							cin >> response;
							counterValue = 0;

							for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
							{
								if (response == to_string(counterValue++))
								{
									cout << "\nValueID \"" << Manager::Get()->GetValueLabel(*it2)
										 << "\", current: " << to_string(Manager::Get()->GetPollIntensity(*it2))
										 << " poll(s)/interval\n";

									cout << "\nSet intensity (0=none, 1=every time through the list, 2-every other time, 'q' to exit): ";
									cin >> response;

									if (response != "q")
									{
										Manager::Get()->SetPollIntensity(*it2, stoi(response));
										cout << "Done\n";
									}
									break;
								}
							}
						}
					}
				}
				else if (response == "2")
				{
					cout << "/!\\ [PLEASE READ] Set the time period between polls of a node's state. Due to patent concerns, some devices do not report state changes automatically to the controller. These devices need to have their state polled at regular intervals. The length of the interval is the same for all devices. To even out the Z-Wave network traffic generated by polling, OpenZWave divides the polling interval by the number of devices that have polling enabled, and polls each in turn. It is recommended that if possible, the interval should not be set shorter than the number of polled devices in seconds (so that the network does not have to cope with more than one poll per second).\n\nCurrent poll interval: " << Manager::Get()->GetPollInterval() << "ms\n\nSet interval in milliseconds ('q' to exit): ";
					cin >> response;

					if (response != "q")
					{
						Manager::Get()->SetPollInterval(stoi(response), false);
						cout << "Done.\n";
					}
				}
			}
			break;
		case 1:
		{
			// Putting the driver into a listening state
			Manager::Get()->AddNode(Five::homeID, false);

			// Printing progression messages
			thread t3(nodeSwitch, stateInt, &lock);
			t3.detach();
			stateInt = Manager::Get()->GetDriverState(Five::homeID);

			break;
		}
		case 2:
		{
			// Putting the driver into a listening state
			Manager::Get()->RemoveNode(Five::homeID);

			// Printing progression messages
			//  thread t3(statusObserver, nodes);
			//  t3.join();

			// thread t3(nodeSwitch, stateInt, &lock);
			// t3.detach();
			// stateInt = Manager::Get()->GetDriverState(Five::homeID);

			break;
		}
		case 3:
			// Printing node names and receiving user's choice
			nodeChoice(&choice, it);

			if (choice == -1)
			{ // Impossible value except if user chooses to quit
				break;
			}

			// Printing all the node's values with name, id and ReadOnly state
			printValues(&choice, &it, it2, true);
			break;
		case 4:
			// Printing node names and receiving user's choice

			nodeChoice(&choice, it);

			if (choice == -1)
			{ // Impossible value except if user chooses to quit
				break;
			}
			// counterNode = 0;

			// Printing value names and receiving user's choice
			for (it = Five::nodes->begin(); it != Five::nodes->end(); it++)
			{
				counterNode++;
				if ((*it)->m_nodeId == choice)
				{
					for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
					{
						if (!Manager::Get()->IsValueReadOnly(*it2))
						{
							counterValue++;
							cout << counterValue << ". " << Manager::Get()->GetValueLabel(*it2) << endl;
						}
					}

					cout << "\nChoose a valueID: ";
					cin >> response;
					counterValue = 0;
					choice = stoi(response);

					for (it2 = (*it)->m_values.begin(); it2 != (*it)->m_values.end(); it2++)
					{
						// Manager::Get()->GetValueAsString(*valueIt, ptr_container);
						// cout << Manager::Get()->GetValueLabel(*valueIt) << ": " << *ptr_container << endl;
						if (!Manager::Get()->IsValueReadOnly(*it2))
						{
							counterValue++;
						}

						if (choice == counterValue)
						{

							// Printing the current value
							cout << Manager::Get()->GetValueLabel(*it2) << it2->GetAsString() << endl;
							Manager::Get()->GetValueAsString((*it2), ptr_container);
							cout << "Current value: " << *ptr_container << endl;

							// Asking the user to set the wanted value
							cout << "Set to what ? ";
							cin >> response;
							int tempCounter{100};
							// int test = 0;
							// int* testptr = &test;
							// setUnit((*valueIt));

							// Sending the value until current value is identical, or until timeout (for sleeping nodes)
							while (response != *ptr_container && tempCounter-- > 0)
							{
								// if (response.find('.')) {
								// 	float temp = stof(response);
								// 	Manager::Get()->SetValue(*it2, temp);
								// } else {
								// }

								Manager::Get()->SetValue((*it2), response);
								Manager::Get()->GetValueAsString((*it2), ptr_container);
								this_thread::sleep_for(chrono::milliseconds(100));
								cout << "send " << response << ", " << *ptr_container << "\n";
							}

							// Manager::Get()->GetValueAsInt((*valueIt), testptr);
							// cout << *testptr;
							break;
						}
					}
					break;
				}
			}
			// cin >> response;
			// choice = stoi(response);
			// counterNode = 0;
			// counterValue = 0;

			// for(valueIt = (*nodeIt) -> m_values.begin(); valueIt != (*nodeIt) -> m_values.end(); valueIt++)
			// {
			// 	counterValue++;
			// 	if (counterValue == choice)
			// 		{
			// 			Manager::Get()->GetValueAsString(*valueIt, ptr_container);
			// 			cout << "The current value is: " << ptr_container << endl;
			// 			cout << "Enter the new value: " << endl;
			// 			cin >> response;
			// 			Manager::Get()->SetValue(*valueIt, response);
			// 		}
			// }

			break;
		case 5:
			cout << "Choose between: " << endl;
			cout << "1. Hard Reset (Z-Wave Network will be deleted during reset)\n"
				 << "2. Soft Reset (Z-Wave Network will be kept during reset)\n";
			while (!isOk)
			{
				cin >> response;
				choice = stoi(response);
				if (choice == 1)
				{
					Manager::Get()->ResetController(Five::homeID);
					isOk = true;
				}
				else if (choice == 2)
				{
					Manager::Get()->SoftReset(Five::homeID);
					isOk = true;
				}
				else
				{
					cout << "Please enter 1 or 2\n";
				}
			}
			menuRun = 0;
			break;
		case 6:
			break;
		case 7:
			// Printing node names and receiving user's choice
			nodeChoice(&choice, it);

			// Ask the node to recheck his neighboors
			for (it = Five::nodes->begin(); it != Five::nodes->end(); it++)
			{
				if ((*it)->m_nodeId == choice)
				{
					Manager::Get()->HealNetworkNode((*it)->m_homeId, (*it)->m_nodeId, true);
				}
			}
			break;
		case 8:
			// Printing node names and receiving user's choice
			nodeChoice(&choice, it);
			// counterNode = 0;
			if (choice == -1)
			{ // Impossible value except if user chooses to quit
				break;
			}

			// Printing value names and receiving user's choice
			printValues(&choice, &it, it2, false);

			if (choice == -1)
			{ // Impossible value except if user chooses to quit
				break;
			}

			// Calling appropriate method depending on user's choice
			newSetValue(&choice, &it, it2, isOk);
			break;
		default:
			cout << "You must enter 1, 2, 3 or 4." << endl;
			break;
		}

		if (fileName.size() > 0)
		{
			char arr[fileName.length()];
			strcpy(arr, fileName.c_str());
			for (int i = 0; i < int(fileName.length()); i++)
			{
				cout << arr[i];
			}
			cout << endl;
			// int i;
			// int counter{ fileName.size() + 1 };
			// char
			// const char *fileChar = fileName.c_str();
			// cout << (*fileChar)[0] << (*fileChar)[1] << endl;
			// char[counter] fileChar =
			// for (i = 0; i < fileName.size(); i++) {
			// 	fileChar app fileName.at(i);
			// }
		}

		// Manager::Get()->AddNode(Five::homeID, false);
		// Manager::Get()->RemoveNode(Five::homeID);
		// cout << "Node removed" << endl;
		// Manager::Get()->TestNetwork(Five::homeID, 5);
		// cout << "Name: " << Manager::Get()->GetNodeProductName(Five::homeID, 2).c_str() << endl;
	}
}

// Function printing progression messages during Add Node and Remove Node
void Five::nodeSwitch(int stateInt, int *lock)
{
	int counter(500);
	while (counter-- > 0)
	{
		switch (stateInt)
		{
		case 1:
			if (*lock != stateInt)
			{
				cout << "STARTING" << endl;
			}
			*lock = 1;
			break;
		case 4:
			if (*lock != stateInt)
			{
				cout << "WAITING" << endl;
			}
			*lock = 4;
			break;
		case 7:
			if (*lock != stateInt)
			{
				cout << "COMPLETED" << endl;
			}
			*lock = 7;
			break;
		default:
			break;
		}
		this_thread::sleep_for(chrono::milliseconds(20));
	}
	cout << "Done" << endl;
}

// Function looping in the background to check if nodes are failed
void Five::CheckFailedNode(string path)
{
	while (true)
	{
		list<NodeInfo *>::iterator it;
		fstream file;
		bool isIn(0);
		this_thread::sleep_for(chrono::seconds(FAILED_NODE_INTERVAL));

		for (it = n.begin(); it != n.end(); it++)
		{
			// cout << "in node for" << endl;
			uint8 nodeId = (*it)->m_nodeId;
			string line;
			string nodeName = (*it)->m_name;
			string nodeType = (*it)->m_nodeType;
			if (Manager::Get()->IsNodeFailed(homeID, nodeId))
			{
				file.open(path, ios::in);
				while (getline(file, line))
				{
					if (line.find("Label: " + nodeName) != string::npos)
					{
						isIn = 1;
					}
				}
				file.close();
				// cout << "NODE HAS FAILED: " << nodeName << " id: " << unsigned(nodeId) << endl;
				if (!isIn)
				{
					file.open(path, ios::app);
					file << "Label: " << nodeName << " Id: " << unsigned(nodeId) << " Type: " << nodeType << endl;
					file.close();
				}
			}
			else if (!(Manager::Get()->IsNodeFailed(homeID, nodeId)))
			{
				// cout << "node not failed" << "id: " << unsigned(nodeId) << endl;
				file.open(path, ios::in);
				fstream temp;
				temp.open("temp.txt", ios::app);
				while (getline(file, line))
				{
					cout << line << endl;
					string s = "Label: " + nodeName;
					// cout << s << endl;
					if (line.find(s) == string::npos)
					{
						temp << line;
					}
				}
				temp.close();
				file.close();

				const char *p = path.c_str();
				remove(p);
				rename("temp.txt", p);
			}
		}
	}
}