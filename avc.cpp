#include <iostream>
#include "E101.h"
#include <cmath>
// #include <math.h>
#include <vector>
#include <cstdlib>

using namespace std;

static double SPEED = 50;          // do not change
static double distanceTune = 3400; // how far it moves per cm
static double angleTune = 600.0;   // how much it moves per degree //600

enum ColourType
{
    RED = 1,
    GREEN,
    BLUE,
    BALL,
    BLACK_LINE
};

// struct return{ to be added

void motorSet(int motorNum, double Speed)
{
    double MotorSpeed;
    if (motorNum == 3 && Speed > 0)
    { // right side forwards -0.98
        MotorSpeed = ((Speed * -0.85) / (100 / 17.5)) + 47.5;
    }
    if (motorNum == 5 && Speed > 0)
    { // left side forwards 0.55
        MotorSpeed = ((Speed * 0.65) / (100 / 17.5)) + 47.5;
    }
    if (motorNum == 3 && Speed < 0)
    { // right side backwards -1.0
        MotorSpeed = ((Speed * -1.0) / (100 / 17.5)) + 47.5;
    }
    if (motorNum == 5 && Speed < 0)
    { // left side backwards 0.85
        MotorSpeed = ((Speed * 0.85) / (100 / 17.5)) + 47.5;
    }
    if (Speed == 0)
    {
        MotorSpeed = (Speed / (100 / 17.5)) + 47.5;
    }
    if (motorNum == 1)
    {
        MotorSpeed = (Speed / (100 / 17.5)) + 47.5;
    }
    set_motors(motorNum, int(MotorSpeed));
}

void moveForward()
{
    motorSet(3, SPEED);
    motorSet(5, SPEED);
    hardware_exchange();
}

void moveBackwards()
{
    motorSet(3, -SPEED);
    motorSet(5, -SPEED);
    hardware_exchange();
}

void Stop()
{
    motorSet(3, 0);
    motorSet(5, 0);
    hardware_exchange();
}

void turnAngle(double angle)
{                                                    // runs the motors for the duration needed to turn an inputted angle
    double time = (fabs(angle) * angleTune) / SPEED; // calculates how long is needed, fabs is an absolute value function
    if (angle > 0)
    { // if angle is positive, turns right.
        motorSet(3, -SPEED);
        motorSet(5, SPEED);
    }
    if (angle < 0)
    { // if angle is negative, turns left.
        motorSet(3, SPEED);
        motorSet(5, -SPEED);
    }
    hardware_exchange();
    sleep1(int(time)); // runs the motors for the duration needed to turn a inputted angle
    Stop();
}

void moveDistance(double cm)
{
    double time = (fabs(cm) * distanceTune) / SPEED; // runs the motors for the duration needed to move an inputted distance
    if (cm > 0)
    {
        moveForward();
    }
    if (cm < 0)
    {
        moveBackwards();
    }
    sleep1(int(time));
    Stop();
}

bool isRed()
{
    int redPix = 1;
    double colourRatio = 1.8; // how much more does red need to be compared to blue and green for it to be classified a red pixel.
    int minRedLevel = 50;     // how many red pixels are needed for the camera to declare red detected
    for (int row = 0; row < 30; row++)
    {
        for (int col = 0; col < 320; col++)
        {

            int Red = (int)get_pixel(row, col, 0);
            int Green = (int)get_pixel(row, col, 1);
            int Blue = (int)get_pixel(row, col, 2);
            int Alpha = (int)get_pixel(row, col, 3);

            if (Alpha > 25 && Red > colourRatio * Blue && Red > colourRatio * Green)
            { // Alpha must be above 25 to not accidentally find red in black ie. R=2 G=1 B=1
                redPix++;
            }
        }
    }
    if (redPix > minRedLevel)
    {
        return true;
    }
    return false;
}

int findPosition(int startRow, int endRow, int startColumn, int endColumn)
{
    take_picture();
    update_screen();
    vector<int> line;
    vector<int> index;
    int lineMin = 25;  // the minimum number of pixels to define a black line
    int lineMax = 100; // the maximum number of pixels to define a black line 140
    int count = 1;     // the total number of black pixels in the scan
    int sum = 0;       // the total value of offset from the middle for each pixel
    int minBlackValue = 100;
    int maxBlackValue = 1;
    // int minBlackValue = threshold;

    // the min number of black pixels needed for a line to be present

    // Finds the darkest pixel in the scan and sets the threshold to this value + a margin.
    for (int row = startRow; row <= endRow; row++)
    { // for each pixel in the line set:
        for (int col = startColumn; col <= endColumn; col++)
        {
            int currentPixel = (int)get_pixel(row, col, 3);

            if (currentPixel < minBlackValue)
            { // if the pixel Alpha value is below the threshold, then it is black.
                minBlackValue = currentPixel;
            }
            if (currentPixel > maxBlackValue)
            { // if the pixel Alpha value is below the threshold, then it is black.
                maxBlackValue = currentPixel;
            }
        }
    }
    // cout << "minlev " << minBlackValue <<"   maxlev " << maxBlackValue << endl;

    double threshold = minBlackValue + (maxBlackValue - minBlackValue) * 0.3;

    for (int row = startRow; row <= endRow; row++)
    { // for each pixel in the line set:
        for (int col = startColumn; col <= endColumn; col++)
        {
            // cout << (int)get_pixel(row, col, 3) << endl;
            if ((int)get_pixel(row, col, 3) < threshold && maxBlackValue - minBlackValue > 50)
            {                      // if the pixel Alpha value is below the threshold, then it is black.
                line.push_back(1); // if black, adds a 1 to the end of a vector list.
                count++;           // adds one to the count of total black pixels.
            }
            else
            {
                line.push_back(0); // if not black, adds a 0 to the end of a vector list.
            }
        }
    }
    for (int i = 0; i < line.size(); i++)
    {                                           // for each value in the line vector list:
        sum += line[i] * (i - line.size() / 2); // adds the current value(0 or 1) * the difference between the current position and the middle of the list to a sum.
    }

    int position = (count != 0) ? (sum / count) : 999;
    // cout << count << endl;
    if (count >= lineMin && count <= lineMax)
    { // if the number of black pixels is more than x, then a black line is present.
        return position;
    }
    else
    {
        return 999; // else if no line is present, return 999, which means no line.
    }
}

bool findColourCylinder(int colourType)
{
    // Colourtype 1 is red, 2 is green, 3 is blue, 4 is ball (red with a different ending), 5 is black line
    take_picture();
    update_screen();
    int ColourPix = 1; // to avoid /0 errors.
    double colourRatio1;
    double colourRatio2;
    int colorPixMin;
    int colorPixMax;
    int margin;

    if (colourType == 1)
    {                       // Red
        colourRatio1 = 1.8; // Red - blue ratio
        colourRatio2 = 1.8; // Red - green ratio
        colorPixMin = 500;
        colorPixMax = 5500;
    }
    else if (colourType == 2)
    {                        // Green
        colourRatio1 = 1.58; // Green - Blue ratio
        colourRatio2 = 1.25; // Green - Red ratio
        colorPixMin = 500;
        colorPixMax = 5500;
    }
    else if (colourType == 3)
    {                       // BLue
        colourRatio1 = 1.3; // Blue - Red ratio
        colourRatio2 = 1.2; // Blue - Green ratio
        colorPixMin = 500;
        colorPixMax = 5500;
    }
    else if (colourType == 4)
    {                     // Red Ball
        colourRatio1 = 2; // Red - blue ratio
        colourRatio2 = 2; // Red - green ratio
        colorPixMin = 500;
        colorPixMax = 5500;
    }

    double colTotal = 0;
    for (int row = 45; row < 75; row++)
    {
        for (int col = 0; col < 320; col++)
        {

            int Red = (int)get_pixel(row, col, 0);
            int Green = (int)get_pixel(row, col, 1);
            int Blue = (int)get_pixel(row, col, 2);

            if (Red > colourRatio1 * Blue && Red > colourRatio2 * Green && (colourType == 1 || colourType == 4))
            {
                ColourPix++;
                colTotal += col;
            }
            else if (Green > colourRatio1 * Blue && Green > colourRatio2 * Red && colourType == 2)
            {
                ColourPix++;
                colTotal += col;
            }
            else if (Blue > colourRatio1 * Red && Blue > colourRatio2 * Green && colourType == 3)
            {
                ColourPix++;
                colTotal += col;
            }
        }
    }
    // AverageColourX holds the average X value of a Colour position on the screen, i.e.
    // if the Colour cylinder is on the right, it will return a value above 160 (the mid point)
    double averageColourX = (colTotal / ColourPix);
    // The centre variant is the same but it holds it relative to the centre, so if the Colour cylinder
    // is to the left of the centre, it returns a negative value and if to the right, a positive value.
    double averageColourXCentre = (averageColourX - 160);

    cout << "ColourCentre: " << averageColourXCentre << endl;
    // cout<<"colTotal    : "<<colTotal<<endl;
    cout << "ColourPix   : " << ColourPix << endl;

    if (colourType == 4 && ColourPix < colorPixMin)
    {
        return true;
    }
    else if (fabs(averageColourXCentre) > 10)
    {
        turnAngle((averageColourXCentre / 3.5));
    }
    else if ((ColourPix >= colorPixMax) && (colourType != 4))
    {
        cout << "moving back" << endl;
        return true;
    }
    else if (ColourPix > colorPixMax - 2000)
    {
        moveDistance(2);
    }
    else if (ColourPix > colorPixMin)
    {
        moveDistance(15);
    }
    else if (colourType != 4)
    {
        return false;
    }
}

void turnUntilMiddle(double direction)
{
    while (true)
    {
        double angleSensitivity = 8;
        int frontPosition = findPosition(10, 10, 10, 310); // 1-240, 1-320;

        if (fabs(frontPosition) > angleSensitivity)
        {
            if (direction > 0)
            {
                turnAngle(5);
                sleep1(100);
            }
            else if (direction < 0)
            {
                turnAngle(-5);
                sleep1(100);
            }
        }
        else if (fabs(frontPosition) < angleSensitivity)
        {
            break;
        }
    }
}

void turnUntilStraight(int moveStep, double angleSensitivity)
{
    int frontPosition, bottomPosition, rightPosition, leftPosition, rightMiddlePosition, leftMiddlePosition = 0;
    cout << "turnUntilStraight" << endl;
    while (true)
    {
        frontPosition = findPosition(10, 10, 10, 310);         // 1-240, 1-320;
        bottomPosition = findPosition(230, 230, 10, 310);      // 1-240, 1-320;
        rightPosition = findPosition(10, 230, 310, 310);       // 1-240, 1-320;
        leftPosition = findPosition(10, 230, 10, 10);          // 1-240, 1-320;
        rightMiddlePosition = findPosition(10, 230, 220, 220); // 1-240, 1-320;
        leftMiddlePosition = findPosition(10, 230, 100, 100);  // 1-240, 1-320;

        if (frontPosition != 999 && bottomPosition != 999)
        {
            if (frontPosition < bottomPosition + angleSensitivity && frontPosition > bottomPosition - angleSensitivity)
            {
                break;
            }
            else if (frontPosition < bottomPosition + angleSensitivity)
            {
                turnAngle(-moveStep);
            }
            else if (bottomPosition < frontPosition + angleSensitivity)
            {
                turnAngle(moveStep);
            }
        }
        else if (rightPosition != 999 && leftPosition != 999)
        {
            if (rightPosition < leftPosition + angleSensitivity &&
                rightPosition > leftPosition - angleSensitivity)
            {
                break;
            }
            else if (rightPosition < leftPosition + angleSensitivity)
            {
                turnAngle(-moveStep);
            }
            else if (leftPosition < rightPosition + angleSensitivity)
            {
                turnAngle(moveStep);
            }
        }
        else if (rightPosition != 999 && rightMiddlePosition != 999)
        {
            if (rightPosition < rightMiddlePosition + angleSensitivity / 2.0 &&
                rightPosition > rightMiddlePosition - angleSensitivity / 2.0)
            {
                break;
            }
            else if (rightPosition < rightMiddlePosition + angleSensitivity / 2.0)
            {
                turnAngle(-moveStep);
            }
            else if (rightMiddlePosition < rightPosition + angleSensitivity / 2.0)
            {
                turnAngle(moveStep);
            }
        }
        else if (leftPosition != 999 && leftMiddlePosition != 999)
        {
            if (leftPosition < leftMiddlePosition + angleSensitivity / 2.0 &&
                leftPosition > leftMiddlePosition - angleSensitivity / 2.0)
            {
                break;
            }
            else if (leftPosition < leftMiddlePosition + angleSensitivity / 2.0)
            {
                turnAngle(moveStep);
            }
            else if (leftMiddlePosition < leftPosition + angleSensitivity / 2.0)
            {
                turnAngle(-moveStep);
            }
        }
        else
        {
            break;
        }
    }
}

void moveUntilCentre()
{
    cout << "moveUntilCentre" << endl;
    int middlePosition = 0;
    int sensitivity = 15;
    int moveStep = 1;
    while (true)
    {
        middlePosition = findPosition(70, 170, 160, 160); // 1-240, 1-320;
        if (middlePosition > -sensitivity && middlePosition < sensitivity)
        {
            break;
        }
        else
        {
            moveDistance(moveStep);
        }
    }
}

void startUp()
{
    cout << endl
         << "Start up in progress" << endl;

    motorSet(3, 0);
    motorSet(5, 0);
    motorSet(1, -100);
    hardware_exchange();
    sleep1(500);
    motorSet(1, 85);
    hardware_exchange();
    turnUntilStraight(2, 5);
    sleep1(2000);
    cout << endl
         << "Start up in complete" << endl
         << endl;
}

void open_gate()
{
    char IP[] = "130.195.3.53";
    char request[24] = "Please";
    char serverMessage[24];
    connect_to_server(IP, 1024);
    cout << "Connected" << endl;
    send_to_server(request);
    receive_from_server(serverMessage);
    send_to_server(serverMessage);
}

void move_through_gate()
{
    moveDistance(30);
}

void followLineSmoothly()
{

    int frontError, rightError, leftError, bottomError = 0;
    while (true)
    {
        frontError = findPosition(10, 10, 10, 310);    // 1-240, 1-320;
        rightError = findPosition(10, 240, 310, 310);  // 1-240, 1-320;
        leftError = findPosition(10, 240, 10, 10);     // 1-240, 1-320;
        bottomError = findPosition(230, 230, 10, 310); // 1-240, 1-320;

        double kp = 0.3; // max 3
        double dv = 0;

        if (isRed())
        {
            Stop();
            cout << "red detected" << endl;
            break;
        }
        else if (frontError == 999 && rightError == 999 && leftError == 999 && bottomError == 999)
        {
            cout << "No lines Detected" << endl;
            moveDistance(-1);
            cout << "Moving back" << endl;
        }
        else if (frontError == 999)
        {
            moveDistance(-1);
            cout << "Moving back" << endl;
        }
        else
        {
            motorSet(3, 30 - kp * frontError);
            motorSet(5, 30 + kp * frontError);
            hardware_exchange();
        }
    }
}

void follow_line_curvy(int moveStep, double angleAdjust, double angleSensitivity)
{
    int frontPosition, rightPosition, leftPosition, bottomPosition = 0;

    while (true)
    {
        frontPosition = findPosition(10, 10, 10, 310);    // 1-240, 1-320;
        rightPosition = findPosition(10, 240, 310, 310);  // 1-240, 1-320;
        leftPosition = findPosition(10, 240, 10, 10);     // 1-240, 1-320;
        bottomPosition = findPosition(230, 230, 10, 310); // 1-240, 1-320;

        cout << "R " << leftPosition << "   F " << frontPosition << "   R " << rightPosition << "   B " << bottomPosition << endl;

        // if red is present, move to next quadrant.
        if (isRed())
        {
            cout << "red detected" << endl;
            break;
        }
        // else if there is no line on any of the sides, quit the program
        else if (frontPosition == 999 && rightPosition == 999 && leftPosition == 999 && bottomPosition == 999)
        {
            Stop();
            cout << "No lines Detected" << endl;
            // exit(0);
        }
        // if the front position is out of range, turn to correct it.
        else if (fabs(frontPosition) > angleSensitivity && fabs(frontPosition) < 150)
        {
            turnAngle(frontPosition * angleAdjust);
        }
        //
        else if (frontPosition == 999)
        {
            moveDistance(-1);
        }
        else if (fabs(frontPosition) < angleSensitivity)
        {
            // moveForward();
            moveDistance(moveStep);
        }
    }
}

void follow_the_straight_line(int moveStep, double angleAdjust, double angleSensitivity, bool redArmed)
{
    int frontPosition, rightPosition, leftPosition, bottomPosition = 0;

    while (true)
    {
        frontPosition = findPosition(10, 10, 10, 310);    // 1-240, 1-320;
        rightPosition = findPosition(10, 240, 310, 310);  // 1-240, 1-320;
        leftPosition = findPosition(10, 240, 10, 10);     // 1-240, 1-320;
        bottomPosition = findPosition(230, 230, 10, 310); // 1-240, 1-320;

        cout << "L " << leftPosition << "   F " << frontPosition << "   R " << rightPosition << "   B "
             << bottomPosition << endl;
        if (isRed() && redArmed)
        { // if there is no line at top or red is present, more to next quadrant.
            cout << "red detected" << endl;
            break;
        }
        else if (frontPosition == 999 && rightPosition == 999 && leftPosition == 999 && bottomPosition == 999)
        {
            cout << "No lines Detected" << endl;
            exit(0);
        }
        else if (frontPosition == 999 && bottomPosition != 999 && rightPosition != 999 && leftPosition != 999)
        {
            cout << "T Detected" << endl;
            break;
        }
        else if ((frontPosition == 999 && rightPosition != 999) || (frontPosition == 999 && leftPosition != 999))
        {
            cout << "Corner Detected" << endl;
            break;
        }
        else if (frontPosition != 999 && rightPosition != 999 && leftPosition != 999 && bottomPosition != 999)
        {
            cout << "X detected" << endl;
            break;
        }
        else if (fabs(frontPosition) > 200)
        {
            moveDistance(-1);
        }
        else if ((fabs(frontPosition) >= angleSensitivity) && (fabs(frontPosition) < 200))
        {
            turnAngle(frontPosition * angleAdjust);
        }
        else if (fabs(frontPosition) < angleSensitivity)
        {
            moveDistance(moveStep);
        }
    }
}
void cornerTurn(int direction)
{
    moveDistance(-5);
    turnUntilStraight(4, 10);
    turnAngle(30 * direction);
    moveDistance(5);
    moveUntilCentre();
    turnUntilMiddle(direction);
}
void follow_line_sharp()
{

    moveDistance(10);

    follow_the_straight_line(5, 0.2, 9, false);
    cornerTurn(1);

    follow_the_straight_line(5, 0.2, 9, false);
    cornerTurn(1);

    follow_the_straight_line(5, 0.2, 9, false);
    cornerTurn(-1);

    follow_the_straight_line(5, 0.2, 9, false);
    cornerTurn(-1);

    follow_the_straight_line(5, 0.2, 9, false);
    cornerTurn(1);

    follow_the_straight_line(5, 0.2, 9, false);
    cornerTurn(-1);

    follow_the_straight_line(5, 0.2, 9, false);
    cornerTurn(1);

    follow_the_straight_line(5, 0.2, 9, true);
}

int go_to_red_cylinder()
{
    moveDistance(5);
    motorSet(1, -100); // tilts camera up
    turnAngle(50);
    while (!(findColourCylinder(1)))
    {
        sleep1(100);
    }
    return 0;
}

int go_to_green_cylinder()
{
    moveDistance(-10);
    turnAngle(-60);
    while (!(findColourCylinder(2)))
    {
        sleep1(100);
    }
    return 0;
}

int go_to_blue_cylinder()
{
    moveDistance(-10);
    turnAngle(50);
    while (!(findColourCylinder(3)))
    {
        sleep1(100);
    }
    return 0;
}

int ball_push()
{
    moveDistance(-10);
    turnAngle(-50);
    while (!(findColourCylinder(4)))
    {
        sleep1(100);
    }
    Stop();
    return 0;
}

void ending()
{
    moveDistance(-20);

    for (int i = 0; i < 10; i++)
    {
        motorSet(1, -100);
        hardware_exchange();
        sleep1(500);
        motorSet(1, 85);
        hardware_exchange();
        sleep1(500);
    }
}

int getFrontPosition() { return findPosition(10, 10, 10, 310); }
int getRightPosition() { return findPosition(10, 240, 310, 310); }
int getLeftPosition() { return findPosition(10, 240, 10, 10); }
int getBottomPosition() { return findPosition(230, 230, 10, 310); }

void log(const std::string &message)
{
    std::cout << "[LOG] " << message << std::endl;
}

int main()
{
    int err;
    err = init(0);
    cout << "Error: " << err << endl;
    open_screen_stream();
    startUp();
    open_gate();
    move_through_gate();
    followLineSmoothly();
    // follow_line_curvy(5,0.3, 9);
    moveDistance(-3);
    turnUntilStraight(2, 10);
    follow_line_sharp();
    go_to_red_cylinder();
    go_to_green_cylinder();
    go_to_blue_cylinder();
    ball_push();
    ending();
}
