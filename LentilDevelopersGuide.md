# Lentil Developer's Guide

My commercial version of Lentil-BHGC aims to keep Lentil working with recent versions of Arnold and includes improvements and optimizations I've made: faster render times using a better RNG algorithm, purple chromatic abberation, updated UI, Windows installers, Gaffer support, beta 3ds Max support, new documentation, and more. 

That being said, some people are just interested in playing around with the Lentil source and maybe modifying it to suit their individual needs. This article is here to help you understand what has changed in Arnold since Lentil's open source release in 2022 and what new challenges there are!

## Cryptomatte

Lentil uses a lightly modified version of Cryptomatte. (Or it did! See below for 7.3.5.0 news.) There are just a few changes in _cryptomatte.h_ and _cryptomatte_shader.cpp_. Copy and paste the Lentil additions into the latest Psyop CryptomatteArnold source and you'll be good to go.

## Critsec

The first major update to Arnold after Lentil was open-sourced was changes to the _critsec_ code. This is a simple change to make: have a look at the CryptomatteArnold 1.2.1 release and change Lentil to match what they did, updating `AiCritSec` to `AtMutex`.

## Arnold 7.3.0.0

This should build no problem with pre-7.3 Lentil code.

## Arnold 7.3.1.0

Arnold introduced a breaking change here: the old "driver" style Imager doesn't work any more, so updates are needed in _lentil_imager.cpp_. The Arnold docs provide some guidance and thankfully it's not too complicated. Basically, just stick all the old Imager code from `driver_process_bucket` in _lentil_imager.cpp_ into a new `imager_evaluate` section. You'll also need `schedule = AtImagerSchedule::FULL_FRAME;` under `imager_prepare` in order to avoid bucket outlines.

There's a problem though. Even if you get the Imager working, Lentil still crashes. There seems to be a new issue related to the Operator. I'm not sure what's going on here. When rendering from the terminal, _kick_ throws a warning about the Operator in 7.3.1.0 that it didn't in 7.3.0.0. There's no mention of any changes related to this in the Arnold docs. What I did as a workaround was to copy the work I did porting Lentil to Gaffer and remove the Operator entirely. This isn't too difficult. In the `operator_cook` section of _lentil_operator.cpp_ you'll see that the Operator creates all the Lentil AOVs. Those are then read in _lentil.h_ in the function `setup_lentil_aovs`. What you can do is copy all the `operator_cook` code into _lentil.h_ and disable loading and compiling the Operator. 

Removing the Operator may lead to other problems. The creator of Lentil, Zeno, went back and forth on including it. Sometimes it was there, sometimes not. The most recent reinclusion was to fix "an issue where random frames lost motion blur when doing non-IPR renders when used in conjunction with cryptomatte." So, that issue may be back in some edge cases. In my testing, the Operator-less version seems to work well, so you'll have to pick your poison. 

It's entirely possible that the Operator problem in 7.3.1.0 is a bug in the Arnold code and not something that we can expect to be fixed any time soon.

## Arnold 7.3.3.1

7.3.3.0 introduced a bug with `node_update` that broke Lentil. This has been fixed in 7.3.3.1. Eventually the `node_update` code in Lentil will have to be changed, but it's not necessary yet.

## Arnold 7.3.5.0

Cryptomatte is now hard coded into Arnold. This means there is no need for a separate _cryptomatte.dll_ and no need for the Lentil Cryptomatte customizations. In my limited testing this seems to be an improvement in Lentil quality of life. There was code in Lentil dedicated to trying to "time" the loading of Lentil vis-a-vis Cryptomatte. It wasn't great. Things appear to be working perfectly so far and this massively simplifies Lentil building and maintenance.

## Arnold 7.4.0.0

The Arnold 7.4.0.0 SDK includes changes to the imager API. `schedule = AtImagerSchedule::FULL_FRAME` needs to be updated to `properties.schedule_full_frame = true`. I gave the new "run the imager after everything finishes" feature a try with `properties.run_once = true`, but that's not a good user experience. The entire frame renders, then very slowly the Lentil imager get applied on top at the end.
