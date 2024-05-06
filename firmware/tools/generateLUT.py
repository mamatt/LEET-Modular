# This program generates LUT (look-up-table) for ADC to frequency conversion
# v0.1 2021-07-30 Johan von Konow
#

adStart = 1472 #(was 1433)  # -3V (midi 0)
adEnd = 3584 #(was 3687)  # +8V > midi 127 (7.58V)
samplingFrequency = 44400
periodLength = 65536


def ad_2_xStep(ad):
    return (2*periodLength*(55 * pow(2, (ad-128)/192-39/4))/samplingFrequency)


def midi_2_freq(midi):
    return (55*pow(2, ((midi-33)/12)))


def freq_2_xStep(freq):
    return((2*periodLength*freq)/samplingFrequency)


def quantize(ad):
    i = 0
    while (ad > midiStep[i]):
        if(i < len(midiStep)-1):
            i += 1
        else:
            break
    if(ad-midiStep[i-1] > midiStep[i]-ad):
        return(midiStep[i])
#        return(i)
    else:
        return(midiStep[i-1])
#        return(i-1)


lut = []
for x in range(adStart, adEnd + 1):
    lut.append(ad_2_xStep(x))

midiFreq = []
for x in range(0, 127+1):
    midiFreq.append(midi_2_freq(x))

midiStep = []
for x in range(0, 127+1):
    midiStep.append(freq_2_xStep(midi_2_freq(x)))

q_lut = []
for x in range(adStart, adEnd + 1):
    q_lut.append(quantize(ad_2_xStep(x)))

# print(ad_2_xStep(2611.2))

#print("const uint16_t freq["+str(len(lut)) + "] = {" + ', '.join(str(int(x)) for x in lut) + "};")
#print()
#print("const uint16_t qfreq["+str(len(q_lut)) + "] = {" + ', '.join(str(int(x)) for x in q_lut) + "};")

print("const float freq["+str(len(lut)) + "] = {" + ', '.join(str(x) for x in lut) + "};")
#print()
#print("const float qfreq["+str(len(q_lut)) + "] = {" + ', '.join(str(x) for x in q_lut) + "};")

#print(midiStep)
#print(quantize(31137))
#print(quantize(31139))

### test that formula below results in correct quantization (removing need for dedicated q_lut)
#for x in range(adStart, adStart + 20):
#    print("x" + str(x) + ":" + str(lut[int((x-adStart+8)/16)*16]))