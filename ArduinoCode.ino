
//----------global variables and function definitions-----------------
const int mlf = 5;
const int mlb = 4;
const int mrf = 2;
const int mrb = 3;


class junction;
class path;

int initial_motor_speed = 80;
// PID Constants
int Kp = 6;
int Ki = 2;
int Kd = 1;


int error = 0, P = 0, I = 0, D = 0, PID_value = 0;
int previous_error = 0, previous_I = 0;
int dt = 1;

int s[8];//Sensor value taker variable

int jValueCounter=0;
int scanningStackCounter=1;int scanningStackCurrent=1;
int endFound=0;
int newJId,currentJId;
int endId=0;//the stops's id 
int obstacleFound=0;
int superArray[8][10];
int k=1;



int foundJunctionInitiation;
unsigned long unitTimeStraight=1000;
unsigned long unitTimeDiagonal=1000;
unsigned long oldTime,newTime;

void initialStart();
int maxi(int,int);
int absi(int);
void ledBlink(int);
int moveForwardLittle(int );
void forward();
void calculate_pid();
void motor_control();
void moveTillNextJunctionNotFound();
void moveTillNextJunctionNotFoundSolver();

int absoluteDistanceBetween(int a,int b);
void findShortestPath(int fromId,int toId);
void followShortestPath(int ,int);
void updateScanningStack(int ,int);
void readSensorData(int []);
void showSensorData(int [] );
void setup();
void loop();
void scanMaze();
void moveStraight();
int identifyJunction(int ,unsigned long timeTaken);//shoould return the id value
boolean compareSensorData(int,int,int,int,int,int,int,int);
void updateBotDistance(long timeTaken);
int alreadyExists(int x,int y);


//----------------global variables end----------------

//-------------------------classes start------------------
class node{
    public:
    int id;
    int data;
    node*next=NULL;
    node*prev=NULL;
    node(){
        id=data=0;
        next=prev=NULL;
    }
};
class myqueue{//start------------------>front
    public:
    node* front=NULL;
    node* start=NULL;
    node * newptr=NULL;
    int length;
    int idTracer;
    myqueue(){
        front=NULL;
        start=NULL;
        newptr=NULL;
        length=0;
        idTracer=0;
    }
    void add(int n){
        newptr=new node();
        newptr->data=n;
        newptr->id=idTracer++;
        newptr->next=NULL;
        newptr->prev=NULL;
        if(start==NULL||front==NULL){
            start=newptr;
            front=newptr;
        }
        else{
            front->next=newptr;
            newptr->prev=front;
            front=newptr;
        }
        length++;

    }

    void remove(node* current){
        if(current==start&&current==front){
            delete current;
            start=front=NULL;
        }
        else if(current==start){
            ////cout<<"its removing for"<<current->id;
            node* tmp=start;
            start=start->next;
            
            //delete tmp;
            start->prev=NULL;
            
            
        }
        else if(current==front)
        {
            ////cout<<"its removing for"<<current->id;
            
            front=front->prev;
            front->next=NULL;
            delete current;
            
            
        }
        else{
            ////cout<<"its working for"<<current->id;
            current->prev->next=current->next;
            current->next->prev=current->prev;
            delete current;
            
        } 
        length--;
    }
    void showData(){
        node *tmp=start;
        int i=0;
        while(i<length){//||tmp!=start){
            Serial.print(tmp->data);
            Serial.print("->");
            //cout<<tmp->data<<"->";
            tmp=tmp->next;
            i++;
        }
        //cout<<"\n";
        Serial.print("\n");
    }
    void showId(){
        node *tmp=start;
        while(tmp!=NULL){
            //cout<<tmp->id<<"->";
            tmp=tmp->next;
            
        }
        //cout<<"\n";
    }


    node* removeNodes(int num){
        node* ptr = start;
        while(ptr!=NULL){
            
            if(ptr->data==num){
                //cout << "Found This";
                return ptr;
                
            }
            ptr = ptr->next;
        }
        //cout<<"\n";
    }
    int isThere(int k){
        for(node* tmp=start;tmp!=NULL;tmp=tmp->next){
            if(tmp->data==k){
                //cout<<"Its there";
                return 1;
            }
            
        }
        //cout<<"Its not there";
    }
    void merge(myqueue q2){
        front->next=q2.start;
        q2.start->prev=front;
        front=q2.front;
        
    }
    void makeCircular(){
        //do only when queue isnt NULL
        front->next=start;
        start->prev=front;
    }
    void doNotMakeCircular(){//do only when queue isnt NULL
        front->next=NULL;
        start->prev=NULL;
    }




    
}scanningStack[20];

class path{
    public:
    int beenThrough;
    int endId1;
    int endId2;
    int exists;
    long timeTaken;
    int obstacleExists;

    
    void showAllData();
    path(){
        exists=0;
        beenThrough=0;
        timeTaken=0;
        obstacleExists=0;
    }
};
void path::showAllData(){
    Serial.print("\n");
    Serial.print(beenThrough);
    Serial.print(exists);
    Serial.print(endId1);
    Serial.print(endId2);

}



class junction{
    public:
    int id;//Simportant id can be removed for 1% space
    int noOfPaths;
    int weight;int visited;//for maze solving
    int tellNoOfPathsCovered();
    int showAllDetails();
    //int hasBeenCounted;
    path p[8];//MAKE 'BOTHWAYS COMPATIBLE //VERY VERY IMPORTANT
    int xCoordi,yCoordi;
    junction(){
        id=0;
        //hasBeenCounted=0;
        weight=1000;
        visited=0;
    }
}jArray[50];
int junction::showAllDetails(){
    Serial.print("id:");
    Serial.print(id);
    Serial.print("\nx.y:");
    
    Serial.print(xCoordi);
    Serial.print(",");
    Serial.print(yCoordi);
    Serial.print("\nNo of paths");
    Serial.print(noOfPaths);
    Serial.print("\nNo of paths covered");
    Serial.print(tellNoOfPathsCovered());
    Serial.print("\nPath related things\nBeentrought:Exists:End1:End2");
    for(int i=0;i<8;i++){
        p[i].showAllData();
    }
}
int junction::tellNoOfPathsCovered(){//just for verifying purposes
    int counter =0;
    for(int i=0;i<8;i++){
        if(p[i].beenThrough==1)
        counter++;
    }
    return counter;
}
class bot{
    public:
    int orientation;
    int xCoordi,yCoordi;
    void rotate(int);
    void bot::junctionMovement(int currentJId);
    bot(){
     orientation=0;
    }
}theBot;



void bot::rotate(int newOrientation ){
// put some code here
    if(orientation==(newOrientation+2)%8){
        //left 90 degree
        int i=1000;
        while(i>0){
            analogWrite(mlf, 0);
            analogWrite(mlb, 200);
            analogWrite(mrf, 200);
            analogWrite(mrb, 0);
            i--;
        }
        // analogWrite(mlf, 0);
        // analogWrite(mrf, 0);
        // analogWrite(mlb,0);
        // analogWrite(mrb,0);
        // i=2000;
        i=1000;
        readSensorData(s);
        while(i>0||(s[1]==1||s[2]==1)){
            analogWrite(mlf, 0);
            analogWrite(mlb, 70);
            analogWrite(mrf, 80);
            analogWrite(mrb, 0);
            readSensorData(s);
            i--;
        }
        analogWrite(mlf, 0);
        analogWrite(mlb, 0);
        analogWrite(mrf, 0);
        analogWrite(mrb, 0);
    }
    else if(orientation==(newOrientation+1)%8){
        //left 45 degree
        int i=3000;
        while(i>0){
            analogWrite(mlf, 0);
            analogWrite(mlb, 100);
            analogWrite(mrf, 0);
            analogWrite(mrb, 100);
            i--;
        }
        // analogWrite(mlf, 0);
        // analogWrite(mrf, 0);
        // analogWrite(mlb,0);
        // analogWrite(mrb,0);
        // i=1000;
        readSensorData(s);
        while(i>0||(s[2]==1||s[3]==1)){
            analogWrite(mlf, 0);
            analogWrite(mlb, 70);
            analogWrite(mrf, 80);
            analogWrite(mrb, 0);
            readSensorData(s);
            i--;
        }
        analogWrite(mlf, 0);
        analogWrite(mrf, 0);
        analogWrite(mrb, 0);
        analogWrite(mlb,0);
    }
    else if(newOrientation==(orientation+1)%8){
        //right 45 degree
        int i=3000;
        while(i>0){
            analogWrite(mlf, 0);
            analogWrite(mlb, 100);
            analogWrite(mrf, 0);
            analogWrite(mrb, 100);
            i--;
        }
         analogWrite(mlf, 0);
         analogWrite(mrf, 0);
         analogWrite(mlb,0);
         analogWrite(mrb,0);
         //i=1000;
        readSensorData(s);
        while(i>0||(s[4]==1||s[5]==1)){
            analogWrite(mlf, 100);
            analogWrite(mlb, 0);
            analogWrite(mrf, 0);
            analogWrite(mrb, 100);
            readSensorData(s);
            i--;
        }
        analogWrite(mlf, 0);
        analogWrite(mlb, 0);
        analogWrite(mrf, 0);
        analogWrite(mrb, 0);
    }
    else if(newOrientation==(orientation+2)%8){
        //right 90 degree
        int i=1000;
        while(i>0){
            analogWrite(mlf, 200);
            analogWrite(mlb, 0);
            analogWrite(mrf, 0);
            analogWrite(mrb, 200);
            i--;
        }
        // analogWrite(mlf, 0);
        // analogWrite(mrf, 0);
        // analogWrite(mlb,0);
        // analogWrite(mrb,0);
         //i=2000;
         i=1000;
        readSensorData(s);
        while(i>0||(s[6]==1||s[7]==1)){
            analogWrite(mlf, 50);
            analogWrite(mlb, 0);
            analogWrite(mrf, 0);
            analogWrite(mrb, 90);
            readSensorData(s);
            i--;
        }
        analogWrite(mlf, 0);
        analogWrite(mlb, 0);
        analogWrite(mrf, 0);
        analogWrite(mrb, 0);
        
    }
    else if(newOrientation==(orientation+4)%8){
        //right 180 turn
        int i=1000;
        while(i>0){
            analogWrite(mlf, 200);
            analogWrite(mlb, 0);
            analogWrite(mrf, 0);
            analogWrite(mrb, 200);
            i--;
        }
        // analogWrite(mlf, 0);
        // analogWrite(mrf, 0);
        // analogWrite(mlb,0);
        // analogWrite(mrb,0);
         i=5000;
        readSensorData(s);
        while(i>0||(s[5]==1||s[6]==1)){
            analogWrite(mlf, 65);
            analogWrite(mlb, 0);
            analogWrite(mrf, 0);
            analogWrite(mrb, 75);
            readSensorData(s);
            i--;
        }
        analogWrite(mlf, 0);
        analogWrite(mlb, 0);
        analogWrite(mrf, 0);
        analogWrite(mrb, 0);
    }
    else if(newOrientation==orientation){
        //straignt forward
    }
    orientation=newOrientation;
    //we update the bots orientation in here as we have done above here
    
}
void bot::junctionMovement(int currentJId){
    //change orientation and make note of it;
    if(jArray[currentJId].noOfPaths<=2){
        //trivial code
        int orient=orientation;
        int i;
        for( i=1;i<=8;i++){
            if(jArray[currentJId].p[(orient+4+i)%8].exists==1/*&&jArray[currentJId].p[(orient+4+i)%8].beenThrough==0*/&&jArray[currentJId].p[(orient+4+i)%8].obstacleExists==0){
                Serial.print("breaking at 1");
                Serial.print(i);
                break;
            }
        }
        theBot.rotate((orient+4+i)%8);//it also updates the orientation
        //SimportantjArray[currentJId].p[theBot.orientation].beenThrough=1;
        
    }
    else{
        if(jArray[currentJId].tellNoOfPathsCovered()==jArray[currentJId].noOfPaths){
            if(endFound==1&&scanningStack[scanningStackCurrent].length==0){
                //scanning complete we should possible use return statement
                Serial.print("Scanning Complete");
                // while(true){
                //     digitalWrite(13,HIGH);
                // }
            }
            else{
                Serial.print("breaking at 2");
                findShortestPath(currentJId,scanningStack[scanningStackCurrent].front->data);
                followShortestPath(currentJId,scanningStack[scanningStackCurrent].front->data);
                
                Serial.print("finally we have reached id:");
                Serial.print(currentJId);
                jArray[::currentJId].showAllDetails();
                theBot.junctionMovement(::currentJId);
                
            }
        }
        else{
            int orient=orientation;
            int i;
            for( i=1;i<=8;i++){
                if(jArray[currentJId].p[(orient+4+i)%8].exists==1&&jArray[currentJId].p[(orient+4+i)%8].beenThrough==0&&jArray[currentJId].p[(orient+4+i)%8].obstacleExists==0){
                    Serial.print("breaking at 3");
                    Serial.print(i);
                    break;
                }
            }
            theBot.rotate((orient+4+i)%8);//it also updates the orientation
            //jArray[currentJId].p[theBot.orientation].beenThrough=1;
        }
    }
}


//------------------------classes end-----------------------------

//------------------------functions start-------------------------
void initialStart(){
    int i=1000;
    while(i>0){
    analogWrite(mlf, 200);
    analogWrite(mrf, 200);
    i--;
    }
    analogWrite(mlf, 0);
    analogWrite(mrf, 0);
}
int maxi(int x,int y){
    return ((x>y)?x:y);
}
int absi(int x){
    return ((x>0)?x:(-x));
}
void ledBlink(int a){
    for(int i=0;i<a;i++){
        digitalWrite(13,HIGH);
        delay(500);
        digitalWrite(13,LOW);
        delay(500);
    }
}
int moveForwardLittle(int a){
    int i=20;
    int flaga=0;
    int flagb=0;
    int flagc=0;
    int flagd=0;
    if(a==0){
        while(i>0){
            readSensorData(s);
            if(/*s[7]==0&&s[6]==0&&s[0]==1)*/compareSensorData(1,1,1,1,1,1,1,0)||compareSensorData(1,1,1,1,1,1,0,0)){
                flagb=1;
            }
            if(/*s[0]==0&&s[1]==0&&s[7]==1*/compareSensorData(0,1,1,1,1,1,1,1)||compareSensorData(0,0,1,1,1,1,1,1)){
                flaga=1;
            }
            if(compareSensorData(0,0,0,0,0,0,0,0)){
                flagc=1;
            }
            if((s[0]==0||s[1]==0||s[2]==0)&&(s[5]==0||s[6]==0||s[7]==0)&&(s[3]==1||s[4]==1)){
                flagd=1;
            }
            showSensorData(s);
            forward();
            i--;
        }
        analogWrite(mlf, 0);
        analogWrite(mlb, 0);
        analogWrite(mrf, 0);
        analogWrite(mrb, 0);
        if(flagc==1){
            return 1;
        }
        
        else if(flagd==1){
            return 4;
        } 
        
        else if(flagb==1){
            return 2;
        }
        
        
        else if(flaga==1){
            return 5;
        }
        
        
        else return 0;
    }
    else if(a==7){
        while(i>0){
            readSensorData(s);
            if(/*s[7]==0&&s[6]==0&&s[0]==1)*/compareSensorData(1,1,1,1,1,1,1,0)||compareSensorData(1,1,1,1,1,1,0,0)){
                flagb=1;
            }
            if(/*s[0]==0&&s[1]==0&&s[7]==1*/compareSensorData(0,1,1,1,1,1,1,1)||compareSensorData(0,0,1,1,1,1,1,1)){
                flaga=1;
            }
            if(compareSensorData(0,0,0,0,0,0,0,0)){
                flagc=1;
            }
            if((s[0]==0||s[1]==0||s[2]==0)&&(s[5]==0||s[6]==0||s[7]==0)&&(s[3]==1||s[4]==1)){
                flagd=1;
            }
            showSensorData(s);
            forward();
            i--;
        }
        analogWrite(mlf, 0);
        analogWrite(mlb, 0);
        analogWrite(mrf, 0);
        analogWrite(mrb, 0);
        if(flagc==1){
            return 1;
        }
        else if(flagd==1){
            return 4;
        }
        
        else if(flaga==1){
            return 3;
        }
        else if(flagb==1){
            return 6;
        }
        
        
        
        else
        return 0;
    }
    else{
        while(i>0){
            readSensorData(s);
            forward();
            showSensorData(s);
            i--;
        }
        analogWrite(mlf, 0);
        analogWrite(mlb, 0);
        analogWrite(mrf, 0);
        analogWrite(mrb, 0);
    }
    
    
    
}
void forward()
{
 
    analogWrite(mlf, initial_motor_speed);
    analogWrite(mlb, 0);
    analogWrite(mrf, initial_motor_speed - 7);
    analogWrite(mrb, 0);
}
void motor_control()
{
    // Calculating the effective motor speed:
    int left_motor_speed = initial_motor_speed + PID_value;  // NOTE -- for postive error bot needs to be shifted left. so speed of left wheel need to be decreased.
    int right_motor_speed = initial_motor_speed +9 - PID_value;

    analogWrite(mlf, left_motor_speed); //Left Motor Speed
    analogWrite(mrf, right_motor_speed); //Right Motor Speed

    //fol0ing lines of code are to make the bot move forward
    forward();
}
void calculate_pid(){
    readSensorData(s);
    if(false){//detetion of line at both ends
            newTime=millis();
            analogWrite(mlf, 0);
            analogWrite(mlb, 0);
            analogWrite(mrf, 0);
            analogWrite(mrb, 0);
            Serial.print(newTime-oldTime);
            Serial.print("\n");
            delay(1000);
    }
    else 
    {
        if((s[2] == 1) && (s[3] == 1) && (s[4] == 1) && (s[5] == 1)){
//            newTime=millis();
//            analogWrite(mlf, 0);
//            analogWrite(mlb, 0);
//            analogWrite(mrf, 0);
//            analogWrite(mrb, 0);
//            Serial.print(newTime-oldTime);
//            Serial.print("\n");
//            delay(1000);
              forward();
        }
        else if ((s[2] == 0) && (s[3] == 1) && (s[4] == 1) && (s[5] == 1)){     //turn robot slghtly left
            error = 3;
            analogWrite(mrf, 75);            
            analogWrite(mlf, 30);
            Serial.print("we are here3\n");
        }
        else if ((s[2] == 0) && (s[3] == 0) && (s[4] ==1 ) && (s[5] == 1)){ // turn robot slightly slightly left
            error = 2;
            analogWrite(mrf, 75);
            
            analogWrite(mlf, 40);
            Serial.print("we are here2\n");
        }
        else if ((s[2] == 1) && (s[3] == 0) && (s[4] == 1) && (s[5] == 1)){
            error = 1;
            analogWrite(mrf, 65);
            
            analogWrite(mlf, 30);
            Serial.print("we are here1\n");
        }
        else if ((s[2] == 1) && (s[3] == 0) && (s[4] == 0) && (s[5] == 1)){ // move straight
            error = 0;
            Serial.print("we are here0\n");
        }
        else if ((s[2] == 1) && (s[3] == 1) && (s[4] == 0) && (s[5] == 1)){ 
            error = -1;
            analogWrite(mrf, 40);
            
            analogWrite(mlf, 50);
            Serial.print("we are here -1\n");
        }
        else if ((s[2] == 1) && (s[3] == 1) && (s[4] == 0) && (s[5] == 0)){ // turn slightly slightly right
            error = -2;
            analogWrite(mrf, 35);
            
            analogWrite(mlf, 60);
            Serial.print("we are here -2\n");
        }
        else if ((s[2] == 1) && (s[3] ==1) && (s[4] == 1) && (s[5] == 0)){ // turn slightly right
            error = -3;
            analogWrite(mrf, 20);
            
            analogWrite(mlf, 55);
            Serial.print("we are here -3\n");
        }
        P = error;
        I = I + previous_I * dt;
        D = (error - previous_error) / dt;

        //PID_value = (Kp * P) + (Ki * I) + (Kd * D);
        
        previous_I = I;
        previous_error = error;
    }
}
void moveTillNextJunctionNotFoundSolver(){
    delay(500);
    initialStart();
    readSensorData(s);
    //Serial.print("we are here2");
    oldTime=millis();
    while(!(s[0]==0||s[7]==0)){
        if(compareSensorData(1,1,1,1,1,1,1,1)){
            break;
        }
        readSensorData(s);
        ///Serial.print("we are here3");
        calculate_pid();
        
        motor_control();
    }
    //when we get out of this while loop means 
    //we have achieved a junction
    
    
    analogWrite(mlf, 0);
    analogWrite(mlb, 0);
    analogWrite(mrf, 0);
    analogWrite(mrb, 0);
    showSensorData(s);
    
    Serial.print("we are stopping over here1\nAnd the time is: ");
    
    //ndelay(1000);
    
    newTime=millis();
    Serial.print(newTime-oldTime);
    Serial.print("\n");
    theBot.xCoordi=jArray[currentJId].xCoordi;
    theBot.yCoordi=jArray[currentJId].yCoordi;
    moveForwardLittle(0);
    
    
    
}
void moveTillNextJunctionNotFound(){
    delay(500);
    readSensorData(s);
    Serial.print("First time read we are here2");
    showSensorData(s);
    
    
    oldTime=millis();
    while(!(s[0]==0||s[7]==0)){
        if(compareSensorData(1,1,1,1,1,1,1,1)){
            break;
        }
        readSensorData(s);
        ///Serial.print("we are here3");
        calculate_pid();
        
        motor_control();
    }
    //when we get out of this while loop means 
    //we have achieved a junction
    
    
    analogWrite(mlf, 0);
    analogWrite(mlb, 0);
    analogWrite(mrf, 0);
    analogWrite(mrb, 0);
    showSensorData(s);
    
    Serial.print("we are stopping over here1\nAnd the time is: ");
    
    //ndelay(1000);
    
    newTime=millis();
    Serial.print(newTime-oldTime);
    Serial.print("\n");
    updateBotDistance(newTime-oldTime);
    int t=alreadyExists(theBot.xCoordi,theBot.yCoordi);
    
    if(t==0){//dosnt already exists
         if(compareSensorData(1,1,1,1,1,1,1,1)){
            initialStart();
            moveForwardLittle(1);
            currentJId= identifyJunction(13,newTime-oldTime);
        }
        
        else if(s[0]==0&&(s[3]==0||s[4]==0)){
            int i=moveForwardLittle(0);
            if(i==1){
                readSensorData(s);
                if(compareSensorData(1,1,1,1,1,1,1,1)){
                    currentJId=  identifyJunction(10,newTime-oldTime);
                }
                else if(compareSensorData(0,0,0,0,0,0,0,0)){
                    currentJId=  identifyJunction(12,newTime-oldTime);
                }
                else{
                    currentJId=  identifyJunction(11,newTime-oldTime);
                }
            }
            else{
                readSensorData(s);
                if(s[3]==0||s[4]==0){
                    currentJId=  identifyJunction(7,newTime-oldTime);
                }
                else if(i==2){
                    currentJId=  identifyJunction(5,newTime-oldTime);
                }
                else if(i==4){
                    currentJId=  identifyJunction(9,newTime-oldTime);
                }
                else if(i==5){
                    currentJId=  identifyJunction(3,newTime-oldTime);
                }
                else if(compareSensorData(1,1,1,1,1,1,1,1)){
                    currentJId=  identifyJunction(1,newTime-oldTime);
                }
                
            }
        }
        
        else if(s[7]==0&&(s[3]==0||s[4]==0)){
            initialStart();
            int i=moveForwardLittle(7);
            if(i==1){
                readSensorData(s);
                if(compareSensorData(1,1,1,1,1,1,1,1)){
                    currentJId=  identifyJunction(10,newTime-oldTime);
                }
                else if(compareSensorData(0,0,0,0,0,0,0,0)){
                    currentJId=  identifyJunction(12,newTime-oldTime);
                }
                else{
                    currentJId=  identifyJunction(11,newTime-oldTime);
                }
            }
            else{
                readSensorData(s);
                
                
                if(s[3]==0||s[4]==0){
                    currentJId=  identifyJunction(8,newTime-oldTime);
                }
                else if(i==3){
                    currentJId=  identifyJunction(6,newTime-oldTime);
                }
                else if(i==4){
                    currentJId=  identifyJunction(9,newTime-oldTime);
                }
                else if(i==6){
                    currentJId=  identifyJunction(4,newTime-oldTime);
                }
                else if(compareSensorData(1,1,1,1,1,1,1,1)){
                    currentJId=  identifyJunction(2,newTime-oldTime);
                }
                
            }
        }
        else if((s[0]==0||s[1]==0||s[2]==0)&&(s[5]==0||s[6]==0||s[7]==0)&&(s[3==1]||s[4]==1)){
            currentJId= identifyJunction(9,newTime-oldTime);
            moveForwardLittle(0);
        }
        else if(compareSensorData(0,1,1,1,1,1,1,1)||compareSensorData(0,0,1,1,1,1,1,1)||compareSensorData(0,0,0,1,1,1,1,1)){
            currentJId=  identifyJunction(3,newTime-oldTime);
            moveForwardLittle(0);
        }
        else if(compareSensorData(1,1,1,1,1,1,1,0)||compareSensorData(1,1,1,1,1,1,0,0)||compareSensorData(1,1,1,1,1,0,0,0)){
            currentJId=  identifyJunction(4,newTime-oldTime);
            moveForwardLittle(0);
        }
    }
    else{//if already existing
        newJId=t;
        int orient=theBot.orientation;
        updateScanningStack(newJId,1);
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;

        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;
        currentJId=newJId;
        
        jArray[newJId].showAllDetails();
        moveForwardLittle(0);
    }
    
    Serial.print("\nthe sacnning stack is with currentscanningstackid:");
    Serial.print(scanningStackCurrent);
    Serial.print("\n");
    scanningStack[scanningStackCurrent].showData();
    theBot.junctionMovement(currentJId);
    
}


void followShortestPath(int fromId,int toId){
    for (int i=0;i<=jValueCounter;i++){
        jArray[i].visited=0;
    }
    while(currentJId!=toId){
        int min=1000;
        int pathTaker;
        //currentjid..visited=1 should be somehow put well
        //found out the path to take
        for(int i=0;i<8;i++){
            if(jArray[currentJId].p[i].exists==1/*&&i!=(theBot.orientation+4)%8*/){
                int nextNodeId=jArray[currentJId].p[i].endId2;
                //nSerial.print("Current id, i, nextnode id");
                //nSerial.print(currentJId);
                //nSerial.print(i);
                //nSerial.print(nextNodeId);
                if(jArray[nextNodeId].visited==0&&nextNodeId!=0){
                    if(jArray[nextNodeId].weight<min){
                        min=jArray[nextNodeId].weight;
                        pathTaker=i;
                    }
                }
            }
            
        }
        //nSerial.print("\n----------------------\npathTaker\n\n\n");
        //nSerial.print(pathTaker);
        jArray[currentJId].visited=1;
        currentJId=jArray[currentJId].p[pathTaker].endId2;
        theBot.rotate(pathTaker);
        scanningStack[scanningStackCurrent].showData();
        updateScanningStack(currentJId,1);
        moveTillNextJunctionNotFoundSolver();

    }
}
int absoluteDistanceBetween(int id1,int id2){
    return maxi(absi(jArray[id1].xCoordi-jArray[id2].xCoordi),absi(jArray[id1].yCoordi-jArray[id2].yCoordi));
}
void findShortestPath(int fromId,int toId){//basically assigning weights to junctions
    for (int i=0;i<=jValueCounter;i++){
        jArray[i].visited=0;
        jArray[i].weight=1000;
    }
    jArray[toId].weight=0;
    int min;
    while(jArray[fromId].visited!=1){
        //finding out min
        min=1000;
        for(int i=1;i<=jValueCounter;i++){
            if(jArray[i].visited==0){
                if(jArray[i].weight<min){
                    min=jArray[i].weight;
                }
            }
            //nSerial.print("min finder id,weight,visited");
            //nSerial.print(i);
            //nSerial.print(" ");
            //nSerial.print(jArray[i].weight);
            //nSerial.print(" ");
            //nSerial.print(jArray[i].visited);
            //nSerial.print("\n");
        }
        //nSerial.print("\nmin");
        //nSerial.print(min);
        //nSerial.print("\n");
        for(int i=1;i<=jValueCounter && jArray[fromId].visited==0;i++){
            if(jArray[i].visited==0&&jArray[i].weight==min){
                jArray[i].visited=1;
                for(int j=0;j<8;j++){
                    if(jArray[i].p[j].exists==1){
                        int nextNodeId=jArray[i].p[j].endId2;
                        if(jArray[nextNodeId].visited==0){
                            
                            int tmpWeight=jArray[i].weight+absoluteDistanceBetween(i,nextNodeId);
                            if(tmpWeight<jArray[nextNodeId].weight){
                                jArray[nextNodeId].weight=tmpWeight;
                            }
                        }
                    }
                    
                }
            }
            //nSerial.print("updated min finder id,weight");
            //nSerial.print(i);
            //nSerial.print(jArray[i].weight);
            //nSerial.print("\n");

        }
        //nSerial.print("----------------------\n");
    }
}

void updateScanningStack(int id,int flag){//FLAG REFERS TO has the node alreday been visited or not
    if(flag==0){//doesn't already exists
        if(jArray[id].noOfPaths>2){
            for(int i=0;i<jArray[id].noOfPaths-2;i++){
                scanningStack[scanningStackCurrent].add(id);
            }
        }
    }
    else{//the case when junction was already scanned
        if(jArray[id].tellNoOfPathsCovered()<jArray[id].noOfPaths&&scanningStack[scanningStackCurrent].isThere(id)==1){//in case the new entrant is just the end the end of the old stack
            if(scanningStack[scanningStackCurrent].front->data==id){
                scanningStack[scanningStackCurrent].remove(scanningStack[scanningStackCurrent].front);
                // if(scanningStack[scanningStackCurrent].length==0){
                //     scanningStackCurrent--;
                //     scanningStackCounter--;
                // }
                while(scanningStack[scanningStackCurrent].length==0){
                    if(scanningStackCurrent==0){
                        break;
                    }
                    scanningStackCurrent--;
                    scanningStackCounter--;
                }
            }
            else{
            // scanningStackCounter++;
                int flag2=0;
                for(node* tmp=scanningStack[scanningStackCurrent].start;tmp!=NULL;tmp=tmp->next){
                    
                    if(flag2==1){
                        scanningStack[scanningStackCurrent].add(tmp->data);
                        scanningStack[scanningStackCurrent-1].remove(tmp);
                    }
                    else{
                        if(tmp->data==id){
                            flag2=1;
                            scanningStackCounter++;
                            scanningStackCurrent++;
                        }
                    }
                }
                node* tmp=scanningStack[scanningStackCurrent-1].front;
                scanningStack[scanningStackCurrent-1].remove(tmp);
            }
        }
        // else if(){
//Simportant will it even be the case??

        // }
        

    }
}
int alreadyExists(int x,int y){
    for(int i=1;i<=jValueCounter;i++){
        if(jArray[i].xCoordi==x&&jArray[i].yCoordi==y){
            return i;
            
        }
    }
    return 0;
}
void updateBotDistance(unsigned long timeTaken){
    if(theBot.orientation%2==0){
        int i;
        for( i=1;unitTimeStraight*i<=timeTaken;i++);//some error adjustments can also be made
        i--;
        if(timeTaken-(unitTimeStraight*i)>(unitTimeStraight/2)){
           i++;    
        }
        if(theBot.orientation==0){
            theBot.yCoordi+=i;
        }
        else if(theBot.orientation==2){
            theBot.xCoordi+=i;
        }
        else if(theBot.orientation==4){
            theBot.yCoordi-=i;
        }
        else if(theBot.orientation==6){
            theBot.xCoordi-=i;
        }
    }
    else{//for diagonal movement
        int i;
        for( i=1;unitTimeDiagonal*i<=timeTaken;i++);//some error adjustments can also be made
        if(timeTaken-(unitTimeDiagonal*i)>(unitTimeDiagonal/2)){
           i++;    
        }
        if(theBot.orientation==1){
            theBot.yCoordi+=i;
            theBot.xCoordi+=i;
        }
        else if(theBot.orientation==3){
            theBot.yCoordi-=i;
            theBot.xCoordi+=i;
        }
        else if(theBot.orientation==5){
            theBot.yCoordi-=i;
            theBot.xCoordi-=i;
        }
        else if(theBot.orientation==7){
            theBot.yCoordi+=i;
            theBot.xCoordi-=i;
        }
    }
}
boolean compareSensorData(int a, int b , int c, int d ,int e ,int f, int g ,int h){
    return (s[0]==a&&s[1]==b&&s[2]==c&&s[3]==d&&s[4]==e&&s[5]==f&&s[6]==g&&s[7]==h);
}
void moveStraight(){
    //analogWrite coding
    analogWrite(mrf,150);
    analogWrite(mlf,150);

}
int identifyJunction(int jType,unsigned long timeTaken){//well do all intialisation for the junctions and all
  //move foorward to identify the junction and 
  //make wheels arrive at the junction point
    //collection of data for 11 conditions
    //check idf the junction is actually new pr not??
    //11 if then else condtions
    // also check if we have encountered the end
    // or a dead end or a obstacle
    Serial.print(jType);
    //ledBlink(jType);
    Serial.print("\n");
    //ndelay(1000);
    

    jValueCounter++;
    jArray[jValueCounter].id=jValueCounter;
    jArray[jValueCounter].xCoordi=theBot.xCoordi;
    jArray[jValueCounter].yCoordi=theBot.yCoordi;
    newJId=jValueCounter;
    Serial.print("The value of currentJId is");
    Serial.print(currentJId);
    Serial.print("\n");
    int orient=theBot.orientation;
    
    if(jType==1){//type 1
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        

        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=0;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=1;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=2;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(6+orient)%8].endId1=newJId;

    }
    else if(jType==2){//type 2
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;

        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=1;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=0;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=2;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(2+orient)%8].endId1=newJId; 
    }
    else if(jType==3){//t3
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=0;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=0;
        jArray[newJId].p[(7+orient)%8].exists=1;
        
        jArray[newJId].noOfPaths=2;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(7+orient)%8].endId1=newJId;
    }
    else if(jType==4){//t4
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=1;
        jArray[newJId].p[(2+orient)%8].exists=0;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=0;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=2;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(1+orient)%8].endId1=newJId;
    }
    else if(jType==5){//t5
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=1;
        jArray[newJId].p[(2+orient)%8].exists=0;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=1;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=3;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(6+orient)%8].endId1=newJId;
        jArray[newJId].p[(1+orient)%8].endId1=newJId;
    }
    else if(jType==6){//t6
        
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=1;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=0;
        jArray[newJId].p[(7+orient)%8].exists=1;
        
        jArray[newJId].noOfPaths=3;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(2+orient)%8].endId1=newJId;
        jArray[newJId].p[(7+orient)%8].endId1=newJId;
    }
    else if(jType==7){//t7
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=1;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=0;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=1;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=3;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(6+orient)%8].endId1=newJId;
        jArray[newJId].p[(0+orient)%8].endId1=newJId;
    }
    else if(jType==8){//t8
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=1;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=1;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=0;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=3;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(2+orient)%8].endId1=newJId;
        jArray[newJId].p[(0+orient)%8].endId1=newJId;
    }
    else if(jType==9){//t9
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=1;
        jArray[newJId].p[(2+orient)%8].exists=0;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=0;
        jArray[newJId].p[(7+orient)%8].exists=1;
        
        jArray[newJId].noOfPaths=3;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(7+orient)%8].endId1=newJId;
        jArray[newJId].p[(1+orient)%8].endId1=newJId;
    }
    else if(jType==10){//t10
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=1;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=1;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=3;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(6+orient)%8].endId1=newJId;
        jArray[newJId].p[(2+orient)%8].endId1=newJId;
    }
    else if(jType==11){//t11
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=1;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=1;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=1;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=4;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        jArray[newJId].p[(6+orient)%8].endId1=newJId;
        jArray[newJId].p[(2+orient)%8].endId1=newJId;
        jArray[newJId].p[(0+orient)%8].endId1=newJId;
    }
    else if(jType==12){//end/t12
        // jArray[currentJId].p[orient].beenThrough=1;
        // jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        // jArray[currentJId].p[orient].endId2=newJId;
        // endId=jValueCounter;
        // jArray[newJId].p[(4+orient)%8].endId2=currentJId;
        
        // jArray[newJId].noOfPaths=1;
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=0;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=0;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=1;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
        endId=newJId;
        endFound=1;
    }
    // else if(true){//obstacle condition wont come here //we have to change some of the code
    //     jArray[currentJId.p[orient].endId2=0;//no path exists
    //     obstacleFound=1;
    // }
    else if(jType==13){//dead end/t13
        // jArray[currentJId].p[orient].beenThrough=1;
        // jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        // jArray[currentJId].p[orient].endId2=newJId;
        // jArray[newJId].p[(4+orient)%8].endId2=currentJId;
        
        // jArray[newJId].noOfPaths=1;
        jArray[currentJId].p[orient].beenThrough=1;
        jArray[newJId].p[(orient+4)%8].beenThrough=1;
        
        jArray[newJId].p[(0+orient)%8].exists=0;
        jArray[newJId].p[(1+orient)%8].exists=0;
        jArray[newJId].p[(2+orient)%8].exists=0;
        jArray[newJId].p[(3+orient)%8].exists=0;
        jArray[newJId].p[(4+orient)%8].exists=1;
        jArray[newJId].p[(5+orient)%8].exists=0;
        jArray[newJId].p[(6+orient)%8].exists=0;
        jArray[newJId].p[(7+orient)%8].exists=0;
        
        jArray[newJId].noOfPaths=1;
        
        jArray[currentJId].p[orient].endId2=newJId;

        jArray[newJId].p[(4+orient)%8].endId2=currentJId;

        jArray[newJId].p[(4+orient)%8].endId1=newJId;
    }
    currentJId=newJId;
    updateScanningStack(currentJId,0);//because its a new junction

    jArray[newJId].showAllDetails();
    return newJId;
    
    
}
void readSensorData(int s[8]){
  s[0]=digitalRead(46);
  s[1]=digitalRead(47);
  s[2]=digitalRead(48);
  s[3]=digitalRead(49);
  s[4]=digitalRead(50);
  s[5]=digitalRead(51);
  s[6]=digitalRead(52);
  s[7]=digitalRead(53);
}
void showSensorData(int s[]){
    for(int i=0;i<8;i++){
         Serial.print(s[i]);
    }
    Serial.print("\n");
}
void scanMaze(){//equivalent of move forward
    
    //some initial moving d=forward required
    do{
        // readSensorData(::s);
        // if(foundJunctionInitiation==0){
        //     if(true){//only left or right value reads
        //         foundJunctionInitiation=1;
        //         newTime=millis();
        //         currentJId=identifyJunction(1,newTime-oldTime);
        //         theBot.junctionMovement(currentJId);//specail care of onstacles program to be included over here
        //     }
        //     else{
        //         moveStraight();
        //         readSensorData(::s);
        //         //make a change of data array of array ie the  super arrray
        //         //do the arrays rnd here
        //         // and come back after it is done
        //     }
        // }
        
        moveTillNextJunctionNotFound();
    } while(endFound==0);
}

//------------------------functions end-------------------------



void setup() {
    
    //ardiuno formalities
    for (int i=2;i<=5;i++){
      pinMode(i, OUTPUT);
    }
    pinMode(13,OUTPUT);
    for (int j=46;j<=53;j++){
      pinMode(j, INPUT);
    }
    Serial.begin(9600);
    Serial.print("we are here");
    //initialise robot with current jjunction
//    junction startJ;
//    jValueCounter++;
//    startJ.id=1;
//    startJ.xCoordi=0;
//    startJ.yCoordi=0;
//    // euclid[jValueCounter].xCoordi=startJ.xCoordi;
//    // euclid[jValueCounter].yCoordi=startJ.yCoordi;
//    startJ.p[0].exists=1;
//    startJ.p[0].endId1=startJ.id;
//    jArray[1]=startJ;//look out at this  initialisation
//    currentJId=jValueCounter;
//    //initialising the scanning stack
//    
//    //intialising the bot
//    theBot.orientation=0;
//    
//    oldTime=millis();
    
    
    jValueCounter++;
    currentJId=jValueCounter;
    jArray[currentJId].id=jValueCounter;
    jArray[currentJId].xCoordi=0;
    jArray[currentJId].yCoordi=0;
    jArray[currentJId].p[0].exists=1;
    jArray[currentJId].p[0].endId1=currentJId;
    theBot.orientation=0;
    ledBlink(3);
    scanMaze();
    for(int i=0;i<20;i++){
        digitalWrite(13,HIGH);
        delay(250);
        digitalWrite(13,LOW);
        delay(250);
    }
    theBot.orientation=0;
    currentJId=1;
    findShortestPath(1,endId);
    followShortestPath(1,endId);
    digitalWrite(13,HIGH);
}

void loop() {
//    readSensorData(s);
//   forward();
//    
//    forward();
    
    //showSensorData(s);
      
}
