/*
 * Copyright (C) 2016 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Nicolo' Genesio <nicolo.genesio@iit.it>
 * CopyPolicy: Released under the terms of the GNU GPL v3.0.
*/

#include <string>
#include <cmath>
#include <algorithm>
#include <numeric>

#include <yarp/rtf/TestCase.h>
#include <rtf/dll/Plugin.h>
#include <rtf/TestAssert.h>


#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>

using namespace std;
using namespace RTF;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;


/**********************************************************************/
class TestAssignmentMotorControl : public yarp::rtf::TestCase
{
private:
        BufferedPort<Bottle> portL, portR;
        int iterations;
        std::vector<double> encL, encR;
public:
    /******************************************************************/
    TestAssignmentMotorControl() :
        yarp::rtf::TestCase("TestAssignmentProducerConsumer"), iterations(1000){ }

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
        RTF_ASSERT_ERROR_IF_FALSE(portL.open(contactL), "Cannot open the test port (left)");
        RTF_ASSERT_ERROR_IF_FALSE(portR.open(contactR), "Cannot open the test port (right)");
        // Connect to read the robot's encoders
        RTF_ASSERT_ERROR_IF_FALSE(NetworkBase::connect("/icubSim/left_arm/state:o", portL.getName()),
                                      "Cannot connect to /icubSim/left_arm/state:o");
        RTF_ASSERT_ERROR_IF_FALSE(NetworkBase::connect("/icubSim/right_arm/state:o", portR.getName()),
                                      "Cannot connect to /icubSim/left_arm/state:o");
        return true;
    }

    /******************************************************************/
    virtual void tearDown() {
        RTF_TEST_REPORT("Tearing down TestAssignmentProducerConsumer");
        NetworkBase::disconnect("/icubSim/left_arm/state:o", portL.getName());
        NetworkBase::disconnect("/icubSim/right_arm/state:o", portR.getName());
        portL.close();
        portR.close();
    }

    /******************************************************************/
    virtual void run()
    {
        unsigned int score=0;
        RTF_TEST_REPORT("Reading data from the encoders...");
        for (int i=0; i<iterations; i++)
        {
            Bottle* botL = portL.read();
            Bottle* botR = portR.read();
            if (botL && botR)
            {
                encL.push_back(botL->get(2).asDouble());
                encR.push_back(botR->get(2).asDouble());
            }
            else
            {
                RTF_ASSERT_ERROR_IF_FALSE(botL && botR, "Cannot read from the ports");
            }
        }

        double cosSim = cosine_similarity(encL, encR);

        RTF_TEST_REPORT(Asserter::format("The cosine similarity is: %f", cosSim));

        if (cosSim > -1.0)
        {
            RTF_TEST_REPORT("Cosine similarity > -1.0, +1 points");
            score += 1;
            RTF_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/12 ***** ", score));
        }

        if (cosSim > -0.5)
        {
            RTF_TEST_REPORT("Cosine similarity > -0.5, +1 points");
            score += 1;
            RTF_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/12 ***** ", score));
        }

        if (cosSim > 0.0)
        {
            RTF_TEST_REPORT("Cosine similarity > 0.0, +2 points");
            score += 2;
            RTF_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/12 ***** ", score));
        }

        if (cosSim > 0.5)
        {
            RTF_TEST_REPORT("Cosine similarity > 0.5, +4 points");
            score += 4;
            RTF_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/12 ***** ", score));
        }

        double normRatio =  norm(encL)/norm(encR);

        RTF_TEST_REPORT(Asserter::format("The norm ratio is: %f", normRatio));

        if (normRatio < 1.1 && normRatio > 0.9)
        {
            RTF_TEST_REPORT("Norm ratio == 1, +4 points");
            score += 4;
            RTF_TEST_CHECK(score>0,Asserter::format(" ***** Partial score = %d/12 ***** ", score));
        }

        RTF_TEST_CHECK(score>0,Asserter::format("Total score = %d",score));

    }

    /******************************************************************/
    virtual void interrupt() {
        portL.interrupt();
        portR.interrupt();
    }
};

PREPARE_PLUGIN(TestAssignmentMotorControl)
