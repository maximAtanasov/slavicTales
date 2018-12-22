--Copyright (C) 2018 Maxim Atanasov - All Rights Reserved
--You may not use, distribute or modify this code.
--This code is proprietary and belongs to the "slavicTales"
--project. See LICENCE.txt in the root directory of the project.
--
-- E-mail: atanasovmaksim1@gmail.com

setWindowResolution(432, 768)
setFullscreen(false)
setInternalResolution(1080, 1920)
showMouseCursor()
stopMusic();
stopAllSounds();

use("card.lua")

pause = 0

baseCardHeight = 2400

enableLighting(false)
stopAllSounds()
setLevelSize(3000, 3000)
setBackground("table.png")

card1 = card:new(0, baseCardHeight)
card2 = card:new(110, baseCardHeight)
card2:setTexture("tenHeart.png")
card3 = card:new(220, baseCardHeight)
card3:setTexture("threeHeart.png")
card4 = card:new(330, baseCardHeight)
card4:setTexture("queenHeart.png")
card5 = card:new(440, baseCardHeight)
card5:setTexture("sevenHeart.png")
card6 = card:new(550, baseCardHeight)
card6:setTexture("nineHeart.png")
card6:setTexW(500)

cardsInHand = {}

table.insert(cardsInHand, card1)
table.insert(cardsInHand, card2)
table.insert(cardsInHand, card3)
table.insert(cardsInHand, card4)
table.insert(cardsInHand, card5)
table.insert(cardsInHand, card6)

function cardsInHand:update()
    local i = 1
    while i <= 6 do
        self[i]:update()
        if self[i].goingUp == true then
            self[i]:setTexW(500)
            self[i].goingRight = false
            local j = i+1
            while j <= 6 do
                self[j].goingRight = true
                self[j]:setTexW(120)
                j = j+1
            end
            local k = i - 1
            while k >= 1 do
                self[k].goingRight = false
                self[k]:setTexW(120)
                k = k-1
            end
            card6:setTexW(500)
            if keyPressed("SPACE") then
                self[i].play = true
            end
        end
        i = i + 1
    end
end

----@Key
all_buttons_key = "MOUSE1"

----@Audio
all_buttons_sound = "buttonSelected.wav"

function loop()
    cardsInHand:update()
    if keyPressed("RELOAD") then
        unloadLevel("levelCard")
        loadLevel("levelCard")
        startLevel("levelCard")
    end

    if(mouseOver(card4)) then
        log(INFO, "mouseOver")
    end
end