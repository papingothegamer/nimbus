#pragma once

#include "Nodes/Node.h"
#include <cmath>

namespace Nimbus {

class TestToneNode : public Node {
public:
    TestToneNode();
    ~TestToneNode() override = default;

    void prepare(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void process(const ProcessContext& context) override;

private:
    double currentSampleRate = 44100.0;
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    
    const double frequency = 440.0; // A4
    const double level = 0.1;       // -20dBFS approx
};

} // namespace Nimbus
