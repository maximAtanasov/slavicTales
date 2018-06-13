--Copyright (C) 2018 Maxim Atanasov - All Rights Reserved
--You may not use, distribute or modify this code.
--This code is proprietary and belongs to the "slavicTales"
--project. See LICENCE.txt in the root directory of the project.
--
-- E-mail: atanasovmaksim1@gmail.com

setLevelSize(3000, 3000)
setGravity(0)
showMouseCursor()
setBackground("menu.png")
setDarkness(255)
loadLevel("level1") --preload level1

playMusic("AlaFlair.ogg", 100, -1)

--Load the appropriate language configuration
if language == "russian" then
    use("language_russian.lua")
elseif language == "english" then
    use("language_english.lua")
end

darkness = 255
room = "main"
--Buttons and their update functions

--The click key used for all buttons - the following annotion tells our interpreter that
--the string we declare is going to be used as a key

----@Key
all_buttons_key = "MOUSE1"

----@Audio
all_buttons_sound = "buttonSelected.wav"

--New Game button
button_newGame = button:new(50, 500, NEW_GAME_TEXT, FONT, 80)
button_newGame:setMarginX(474)
button_newGame:setMarginY(92)
button_newGame:setClickKey(all_buttons_key)


function button_newGame:onClick()
    startLevel("level1")
end

function button_newGame:onHover()
    if(self.soundPlayed == 1) then
        playSound("buttonSelected.wav", 100, 0)
        self.soundPlayed = 0;
    end
end

function button_newGame:onNothing()
    self.soundPlayed = 1
end


--Settings button
button_settings = button:new(50, 600, SETTINGS_TEXT, FONT, 80)
button_settings:setMarginX(479)
button_settings:setMarginY(98)
button_settings:setClickKey("MOUSE1")

function button_settings:onClick()
    room = "settings"
    button_newGame:hide()
    button_settings:hide()
    button_exit:hide()
    button_back:show()
    button_soundSwitch:show()
    button_fullscreen:show()
    button_switch1:show()
    button_vsync:show()
    button_switch2:show()
    button_switch3:show()
end

function button_settings:onHover()
    if(self.soundPlayed == 1) then
        playSound(all_buttons_sound, 100, 0)
        self.soundPlayed = 0;
    end
end

function button_settings:onNothing()
    self.soundPlayed = 1
end


--Exit button
button_exit = button:new(50, 700, EXIT_TEXT, FONT, 80)
button_exit:setMarginX(224)
button_exit:setMarginY(85)
button_exit:setClickKey(all_buttons_key)

function button_exit:onClick()
    endGame()
end

function button_exit:onHover()
    if(self.soundPlayed == 1) then
        playSound(all_buttons_sound, 100, 0)
        self.soundPlayed = 0;
    end
end

function button_exit:onNothing()
    self.soundPlayed = 1
end


--===================================================================
--SETTINGS

--Buttons and their update functions
button_back = button:new(50, 800, BACK_TEXT, FONT, 70)
button_back:setMarginX(200)
button_back:setMarginY(65)
button_back:hide()
button_back:setClickKey(all_buttons_key)

function button_back:onClick()
    room = "main"
    button_newGame:show()
    button_settings:show()
    button_exit:show()
    button_back:hide()
    button_fullscreen:hide()
    button_soundSwitch:hide()
    button_switch1:hide()
    button_vsync:hide()
    button_switch2:hide()
    button_switch3:hide()
end

function button_back:onHover()
    if(self.soundPlayed == 1) then
        playSound(all_buttons_sound, 100, 0)
        self.soundPlayed = 0;
    end
end

function button_back:onNothing()
    self.soundPlayed = 1
end

--SOUND BUTTON
button_soundSwitch = button:new(50, 500, SOUND_TEXT, FONT, 70)
button_soundSwitch:setMarginX(240)
button_soundSwitch:setMarginY(65)
button_soundSwitch:hide()
button_soundSwitch:setClickKey(all_buttons_key)

function button_soundSwitch:onClick()
    toggleAudio()
    playSound(all_buttons_sound, 100, 0)
end

function button_soundSwitch:onHover()
    if(self.soundPlayed == 1) then
        playSound(all_buttons_sound, 100, 0)
        self.soundPlayed = 0;
    end
end

function button_soundSwitch:onNothing()
    self.soundPlayed = 1
end

--ON LABEL FOR SOUND
button_switch1 = button:new(ON1_OFFSET, 500, ON_TEXT, FONT, 50)
button_switch1:hide()

function button_switch1:update()
    if getVolume() > 0 then
        button_switch1:setText(ON_TEXT)
    else
        button_switch1:setText(OFF_TEXT)
    end
end

--VSYNC button
button_vsync = button:new(50, 600, VSYNC_TEXT, FONT, 70)
button_vsync:setMarginX(VSYNC_MARGINS)
button_vsync:setMarginY(60)
button_vsync:hide()
button_vsync:setClickKey(all_buttons_key)

function button_vsync:onHover()
    if(self.soundPlayed == 1) then
        playSound(all_buttons_sound, 100, 0)
        self.soundPlayed = 0;
    end
end

function button_vsync:onNothing()
    self.soundPlayed = 1
end

--ON SWITCH FOR VSYNC
 
button_switch2 = checkbox:new(450, 620)
button_switch2:hide()
button_switch2:setClickKey(all_buttons_key)
if getVsyncState() then
    button_switch2:check()
else
    button_switch2:uncheck()    
end

function button_switch2:onClick()
    if getVsyncState() then
        vsyncOff()
    else
        vsyncOn()
    end
end

--Fullscreen button

button_fullscreen = button:new(50, 700, FULLSCREEN_TEXT, FONT, 70)
button_fullscreen:setMarginX(FULLSCREEN_MARGINS)
button_fullscreen:setMarginY(60)
button_fullscreen:hide()
button_fullscreen:setText(FULLSCREEN_TEXT)
button_fullscreen:setClickKey(all_buttons_key);

function button_fullscreen:onClick()
    if getFullscreenStatus() == true then
        setFullscreen(false)
    else
        setFullscreen(true)
    end
end

function button_fullscreen:onHover()
    if(self.soundPlayed == 1) then
        playSound(all_buttons_sound, 100, 0)
        self.soundPlayed = 0;
    end
end

function button_fullscreen:onNothing()
    self.soundPlayed = 1
end


--ON SWITCH FOR FULLSCREEN
button_switch3 = button:new(ON3_OFFSET, 700, ON_TEXT, FONT, 50)
button_switch3:hide()

function button_switch3:update()
    if getFullscreenStatus() == true then
        button_switch3:setText(ON_TEXT)
    else
        button_switch3:setText(OFF_TEXT)
    end
end