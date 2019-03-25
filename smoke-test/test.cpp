/*
 * Copyright (C) 2016 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Nicolo' Genesio <nicolo.genesio@iit.it>
 * CopyPolicy: Released under the terms of the GNU GPL v3.0.
*/

#include <string>
#include <cmath>
#include <algorithm>
#include <numeric>

#include <yarp/robottestingframework/TestCase.h>
#include <robottestingframework/dll/Plugin.h>
#include <robottestingframework/TestAssert.h>


#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>

using namespace std;
using namespace robottestingframework;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;


/**********************************************************************/
class TestAssignmentMotorControl : public yarp::robottestingframework::TestCase
{
private:
        BufferedPort<Bottle> portL, portR;
        int iterations;
        std::vector<double> encL, encR;
public:
    /******************************************************************/
    TestAssignmentMotorControl() :
        yarp::robottestingframework::TestCase("TestAssignmentMotorControl"), iterations(1000){ }

    /******************************************************************/

    virtual ~TestAssignmentMotorControl() {
    }

    /******************************************************************/

    double norm(std::vector<double> &v)
    {
        double sum = 0.0;
        for (int i=0; i<v.size(); i++)
        {
            sum += v[i]*v[i];
        }

        return sqrt(sum);
    }

    /******************************************************************/

    double cosine_similarity(std::vector<double>& left, std::vector<double>& right)
    {
        double inProduct = std::inner_product(std::begin(left), std::end(left), std::begin(right), 0.0);
        double normL = norm(left);
        double normR = norm(right);
        return (inProduct)/(normL * normR);

    }

    /******************************************************************/
    virtual bool setup(yarp::os::Property& property) {
        Contact contactL("/TestAssignmentProducerConsumer/left");
        Contact contactR("/TestAssignmentProducerConsumer/right");
        contactL.setTimeout(5.0);
        contactR.setTimeout(5.0);
        // Open ports
        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(portL.open(contactL), "Cannot open the test port (left)");
        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(portR.open(contactR), "Cannot open the test port (right)");
        // Connect to read the robot's encoders
        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(NetworkBase::connect("/icubSim/left_arm/state:o", portL.getName()),
                                      "Cannot connect to /icubSim/left_arm/state:o");
        ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(NetworkBase::connect("/icubSim/right_arm/state:o", portR.getName()),
                                      "Cannot connect to /icubSim/left_arm/state:o");
        return true;
    }

    /******************************************************************/
    virtual void tearDown() {
        ROBOTTESTINGFRAMEWORK_TEST_REPORT("Tearing down TestAssignmentProducerConsumer");
        NetworkBase::disconnect("/icubSim/left_arm/state:o", portL.getName());
        NetworkBase::disconnect("/icubSim/right_arm/state:o", portR.getName());
        portL.close();
        portR.close();
    }

    /******************************************************************/
    virtual void run()
    {
        unsigned int score=0;

        double home_joint2 = 0.0;
        double diff = 0.0;

        bool portsOpened = NetworkBase::exists("/client/input")  &&
                           NetworkBase::exists("/client/output") &&
                           NetworkBase::exists("/server/input");

        ROBOTTESTINGFRAMEWORK_TEST_CHECK(portsOpened, "Checking that all ports has been opend correctly...");

        if (portsOpened)
        {
            ROBOTTESTINGFRAMEWORK_TEST_REPORT("Ports opened correctly, +1 points");
            score += 1;
            ROBOTTESTINGFRAMEWORK_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/14 ***** ", score));
        }

        ROBOTTESTINGFRAMEWORK_TEST_REPORT("Reading data from the encoders...");
        for (int i=0; i<iterations; i++)
        {
            Bottle* botL = portL.read();
            Bottle* botR = portR.read();
            if (botL && botR)
            {
                double angL = botL->get(2).asDouble();
                double angR = botR->get(2).asDouble();

                // Checking that the left arm is actually moving...
                diff = diff + std::fabs(angL - home_joint2);

                encL.push_back(angL);
                encR.push_back(angR);
            }
            else
            {
                ROBOTTESTINGFRAMEWORK_ASSERT_ERROR_IF_FALSE(botL && botR, "Cannot read from the ports");
            }
        }

        ROBOTTESTINGFRAMEWORK_TEST_CHECK(diff > 5.0, "Checking that the left arm is moving...");

        if (diff > 5.0)
        {
            score += 1;
            ROBOTTESTINGFRAMEWORK_TEST_REPORT("The left arm is moving, +1 points");
            ROBOTTESTINGFRAMEWORK_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/14 ***** ", score));
        }

        double cosSim = cosine_similarity(encL, encR);

        ROBOTTESTINGFRAMEWORK_TEST_REPORT(Asserter::format("The cosine similarity is: %f", cosSim));

        if (cosSim > -1.0)
        {
            ROBOTTESTINGFRAMEWORK_TEST_REPORT("Cosine similarity > -1.0, +1 points");
            score += 1;
            ROBOTTESTINGFRAMEWORK_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/14 ***** ", score));
        }

        if (cosSim > -0.5)
        {
            ROBOTTESTINGFRAMEWORK_TEST_REPORT("Cosine similarity > -0.5, +1 points");
            score += 1;
            ROBOTTESTINGFRAMEWORK_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/14 ***** ", score));
        }

        if (cosSim > 0.0)
        {
            ROBOTTESTINGFRAMEWORK_TEST_REPORT("Cosine similarity > 0.0, +2 points");
            score += 2;
            ROBOTTESTINGFRAMEWORK_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/14 ***** ", score));
        }

        if (cosSim > 0.5)
        {
            ROBOTTESTINGFRAMEWORK_TEST_REPORT("Cosine similarity > 0.5, +4 points");
            score += 4;
            ROBOTTESTINGFRAMEWORK_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/14 ***** ", score));
        }

        double normRatio =  norm(encL)/norm(encR);

        ROBOTTESTINGFRAMEWORK_TEST_REPORT(Asserter::format("The norm ratio is: %f", normRatio));

        if (normRatio < 1.1 && normRatio > 0.9)
        {
            ROBOTTESTINGFRAMEWORK_TEST_REPORT("Norm ratio == 1, +4 points");
            score += 4;
            ROBOTTESTINGFRAMEWORK_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/14 ***** ", score));
        }

        ROBOTTESTINGFRAMEWORK_TEST_CHECK(score>0,Asserter::format("Total score = %d",score));

    }

    /******************************************************************/
    virtual void interrupt() {
        portL.interrupt();
        portR.interrupt();
    }
};

ROBOTTESTINGFRAMEWORK_PREPARE_PLUGIN(TestAssignmentMotorControl)
