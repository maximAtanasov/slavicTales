--just don't touch this file, please

currentID = 0 --MUST start at 0

function newID()
    temp = currentID
    currentID = currentID + 1;
    return temp
end

--cleans up current level and start the specifed one
function startLevel(arg)
    startLevelLua(arg)   
    currentID = 0
    error() --a dirty trick, but works
end

--tells if the mouse is over an object
function mouseOver(object)
    if getMouseX() < object:getTexW() + object:getX() and getMouseX() > object:getX() then
        if getMouseY() > object:getY() - object:getTexH() and getMouseY() < object:getY() then
            return true
        end
    end
    return false
end

--default constructor for all objects
function new(self, x, y)
    o = {}  
    setmetatable(o, self)
    self.__index = self
    if(x ~= nil and y ~= nil) then
        --general
        o.ID = newID()
	    createEntity(o.ID)
        setX(o.ID, x)
        setY(o.ID, y)
        
        --texture
        setTexture(o.ID, o.texture)
        setTexW(o.ID, o.texWidth)
        setTexH(o.ID, o.texHeight)
        setVisible(o.ID, o.isVisible)

        --animation
        setSpriteNum(o.ID, o.spriteNum)
        setAnimationNum(o.ID, o.animationNum)
        
        --physics
        setAffectedByPhysics(o.ID, o.affectedByPhysics)
	    setCollisionBox(o.ID, o.offsetColX, o.offsetColY, o.colX, o.colY)
        setMass(o.ID, o.mass)
    end
    return o
end

--genieric game object, from which all objects inherit
object = {
	texture = "";
    offsetColX = 0;
    offsetColY = 0;
	colX = 0;
	colY = 0;
    isVisible = false;
    texWidth = 0;
    texHeight = 0;
    affectedByPhysics = false;
    mass = 0;

    --animations
    spriteNum = 1;
    animationNum = 0
}

--create a new instance of an object (calls the constructor above)
function object:new(x, y)
    return new(self, x, y)
end

--set the x position of an object
function object:setX(x)
    setX(self.ID, x)
end

--set the y position of an object
function object:setY(y)
    setY(self.ID, y)
end

--set the x and y position of an object
function object:setXY(x, y)
	setX(self.ID, x)
	setY(self.ID, y)
end

--get the x corrdinate of an object
function object:getX()
    return getX(self.ID)
end

--get the y corrdinate of an object
function object:getY()
    return getY(self.ID)
end

--tells if an object (without hard collision) is over another object
function object:overObject(other)
    return entityCollides(self.ID, other.ID)
end

--set the Texture width of an object
function object:setTexW(arg)
    setTexW(self.ID, arg)
end

--set the Texture height of an object
function object:setTexH(arg)
    setTexH(self.ID, arg)
end

--get the Texture width of an object
function object:getTexW()
    return getTexW(self.ID)
end

--get the Texture height of an object
function object:getTexH()
    return getTexH(self.ID)
end

--set the horizontal velocity of an object
function object:setVelocityX(arg)
    setVelocityX(self.ID, arg)
end

--set the vertical velocity of an object
function object:setVelocityY(arg)
    setVelocityY(self.ID, arg)
end

--get the horizontal velocity of an object
function object:getVelocityX()
    return getVelocityX(self.ID)
end

--get the vertical velocity of an object
function object:getVelocityY()
    return getVelocityY(self.ID)
end

--set the mass of an object
function object:setMass(arg)
    self.mass = arg;
    setMass(self.ID, arg)
end

--get the mass of an object
function object:getMass()
    return getMass(self.ID)
end

--set the collision box of an object
function object:setCollision(offsetX, offsetY, width, height)
    self.colX = width
    self.colY = height
    setCollisionBox(self.ID, offsetX, offsetY, self.colX, self.colY)
end

--get the Width of the collision box of an object
function object:getColX()
    return getColX(self.ID)
end

--get the Height of the collision box of an object
function object:getColY()
    return getColY(self.ID)
end

--set the Visibility of an object
function object:setVisible(arg)
    self.isVisible = arg;
    setVisible(self.ID, arg)
end

--set weather an object is static (always drawn at the same corrdinates, regardless of camera position)
function object:setStatic(arg)
    setStatic(self.ID, arg)
end

--set the Texture of an object (filepath)
function object:setTexture(arg)
    self.texture = arg
    setTexture(self.ID, arg)
end

--play the corresponding animation of an object (the corresponding index in the spritesheet)
function object:playAnimation(arg)
    setAnimation(self.ID, arg)
end

--delete object
function object:delete()
    setVisible(self.ID, false)
    self:setCollision(0, 0, 0, 0)
    self = nil
end
