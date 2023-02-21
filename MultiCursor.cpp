#include "MultiCursor.h"

MultiCursor::MultiCursor(chai3d::cWorld* a_parentWorld, const double& a_radius) : chai3d::cToolCursor(a_parentWorld) {
    
}

MultiCursor::~MultiCursor() {
}


bool MultiCursor::applyToDevice(chai3d::cVector3d deformableForce) {
    // check if device is available
    if ((m_hapticDevice == nullptr) || (!m_enabled)) { return (chai3d::C_ERROR); }

    // retrieve force values to be applied to device
    chai3d::cVector3d deviceLocalForce = m_deviceLocalForce;
    chai3d::cVector3d deviceLocalTorque = m_deviceLocalTorque;
    double gripperForce = m_gripperForce;


    ////////////////////////////////////////////////////////////////////////////
    // STARTUP MODE: WAIT FOR SMALL FORCE
    ////////////////////////////////////////////////////////////////////////////

    // if forces are already fully engaged, skip this part
    if (!m_forceEngaged)
    {
        // check if waiting for small forces mode is engaged. engage forces otherwise.
        if (m_useWaitForSmallForce)
        {
            // check if desired force is smaller that threshold.
            if (m_deviceLocalForce.length() < m_smallForceThresh)
            {
                // engage forces only if small forces have been maintained for three cycles.
                if (m_smallForceCounter > 3)
                {
                    m_forceEngaged = true;
                    m_smallForceCounter = 0;
                }
                else
                {
                    m_smallForceCounter++;
                }
            }
            else
            {
                m_smallForceCounter = 0;
            }
        }
        else
        {
            m_forceEngaged = true;
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    // STARTUP MODE: FORCE RISE
    ////////////////////////////////////////////////////////////////////////////

    // check if forces are supposed to rise.
    if (m_forceEngaged && m_flagForceRiseActivated)
    {
        if (m_flagForceRiseFirstTime)
        {
            if (m_useForceRise)
            {
                m_forceRiseClock.reset();
                m_forceRiseClock.start();
                m_flagForceRiseFirstTime = false;
            }
            else
            {
                m_flagForceRiseActivated = false;
            }
        }

        // scale forces
        double scale = 1.0;
        double time = chai3d::cClamp(m_forceRiseClock.getCurrentTimeSeconds(), 0.0, m_forceRiseTime);
        if (m_forceRiseTime > chai3d::C_SMALL)
        {
            scale = time / m_forceRiseTime;
            deviceLocalForce.mul(scale);
            deviceLocalTorque.mul(scale);
            gripperForce *= scale;
        }

        // apply 30% maximum damping when forces rise, if and only if, update rate is running above 400 Hz.
        const double MIN_FREQUENCY = 400.0;
        if ((m_freqRead.getFrequency() > MIN_FREQUENCY) && (m_freqWrite.getFrequency() > MIN_FREQUENCY))
        {
            double deviceLinDamping = (1.0 - scale) * 0.2 * m_hapticDevice->getSpecifications().m_maxLinearDamping;
            double deviceAngDamping = (1.0 - scale) * 0.2 * m_hapticDevice->getSpecifications().m_maxAngularDamping;
            double gripperAngDamping = (1.0 - scale) * 0.2 * m_hapticDevice->getSpecifications().m_maxGripperAngularDamping;

            deviceLocalForce = deviceLocalForce - deviceLinDamping * m_deviceLocalLinVel;
            deviceLocalTorque = deviceLocalTorque - deviceAngDamping * m_deviceLocalAngVel;
            gripperForce = gripperForce - gripperAngDamping * m_gripperAngVel;
        }

        // disable force rizing when completed
        if (m_forceRiseClock.getCurrentTimeSeconds() >= m_forceRiseTime)
        {
            m_flagForceRiseActivated = false;
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    // APPLY FORCES
    ////////////////////////////////////////////////////////////////////////////

    // send force commands to haptic device
    if ((m_forceOn) && (m_forceEngaged))
    {
        m_hapticDevice->setForceAndTorqueAndGripperForce(deviceLocalForce + deformableForce,
            deviceLocalTorque,
            gripperForce);
    }
    else
    {
        chai3d::cVector3d nullv3d(0.0, 0.0, 0.0);
        m_hapticDevice->setForceAndTorqueAndGripperForce(nullv3d,
            nullv3d,
            0.0);
    }

    // update frequency counter
    m_freqWrite.signal(1);

    // return success
    return (chai3d::C_SUCCESS);
}