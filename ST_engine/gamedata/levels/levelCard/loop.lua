--Copyright (C) 2018 Maxim Atanasov - All Rights Reserved
--You may not use, distribute or modify this code.
--This code is proprietary and belongs to the "slavicTales"
--project. See LICENCE.txt in the root directory of the project.
--
-- E-mail: atanasovmaksim1@gmail.com


--code is run in a loop inside the main executable

cardsInHand:update()

if keyPressed("RELOAD") then
    unloadLevel("levelCard")
    loadLevel("levelCard")
    startLevel("levelCard")
end