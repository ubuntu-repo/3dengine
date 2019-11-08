////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////
const TILE_SIZE = 64;
const MAP_NUM_ROWS = 11;
const MAP_NUM_COLS = 15;

const WINDOW_WIDTH = MAP_NUM_COLS * TILE_SIZE;
const WINDOW_HEIGHT = MAP_NUM_ROWS * TILE_SIZE;

const MINIMAP_SCALE_FACTOR = 0.3;

const FOV_ANGLE = 60 * (Math.PI / 180);

const WALL_STRIP_WIDTH = 30;
const NUM_RAYS = WINDOW_WIDTH / WALL_STRIP_WIDTH;

////////////////////////////////////////////////////////////////
// Grid map class
////////////////////////////////////////////////////////////////
class Grid {
    constructor() {
        this.walls = [
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
        ];
    }
    hasWallAt(x, y) {
        if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
            return false;
        }
        var mapGridIndexX = Math.floor(x / TILE_SIZE);
        var mapGridIndexY = Math.floor(y / TILE_SIZE);
        return this.walls[mapGridIndexY][mapGridIndexX] != 0;
    }
    draw() {
        for (var i = 0; i < MAP_NUM_ROWS; i++) {
            for (var j = 0; j < MAP_NUM_COLS; j++) {
                var blockX = j * TILE_SIZE;
                var blockY = i * TILE_SIZE;
                stroke("#222");
                fill(this.walls[i][j] == 1 ? '#222' : '#fff');
                rect(
                    MINIMAP_SCALE_FACTOR * blockX,
                    MINIMAP_SCALE_FACTOR * blockY,
                    MINIMAP_SCALE_FACTOR * TILE_SIZE,
                    MINIMAP_SCALE_FACTOR * TILE_SIZE
                );
            }
        }
    }
}

////////////////////////////////////////////////////////////////
// Player class
////////////////////////////////////////////////////////////////
class Player {
    constructor() {
        this.x = WINDOW_WIDTH / 2;
        this.y = WINDOW_HEIGHT / 7;
        this.radius = 2;
        this.turnDirection = 0; // -1 if left, +1 if right
        this.walkDirection = 0; // -1 if back, +1 if front
        this.rotationAngle = Math.PI / 2;
        this.moveSpeed = 3.0;
        this.rotationSpeed = 3 * (Math.PI / 180);
    }
    update() {
        // player will move this far along the current direction
        var moveStep = this.walkDirection * this.moveSpeed;

        // add rotation if player is rotating (player.turnDirection != 0)
        this.rotationAngle += this.turnDirection * this.rotationSpeed;

        // calculate new player position with simple trigonometry
        var newPlayerX = this.x + Math.cos(this.rotationAngle) * moveStep;
        var newPlayerY = this.y + Math.sin(this.rotationAngle) * moveStep;

        // only set new player position if it is not colliding with the map walls
        if (!grid.hasWallAt(newPlayerX, newPlayerY)) {
            this.x = newPlayerX;
            this.y = newPlayerY;
        }
    }
    draw() {
        fill('blue');
        noStroke();
        circle(
            MINIMAP_SCALE_FACTOR * player.x,
            MINIMAP_SCALE_FACTOR * player.y,
            MINIMAP_SCALE_FACTOR * player.radius
        );
        stroke('blue');
        line(
            MINIMAP_SCALE_FACTOR * player.x,
            MINIMAP_SCALE_FACTOR * player.y,
            MINIMAP_SCALE_FACTOR * (player.x + Math.cos(player.rotationAngle) * 10),
            MINIMAP_SCALE_FACTOR * (player.y + Math.sin(player.rotationAngle) * 10)
        );
    }
}

////////////////////////////////////////////////////////////////
// Ray class
////////////////////////////////////////////////////////////////
class Ray {
    constructor(rayAngle) {
        // keep angle between 0 and 360 deg
        rayAngle = normalizeAngle(rayAngle);

        this.rayAngle = rayAngle;
        this.wallHitX = 0;
        this.wallHitY = 0;
        this.distance = 0;
        // angle is facing down if > 0 deg and less than 90 deg (angle increases clockwise)
        this.isRayFacingDown = rayAngle > 0 && rayAngle < Math.PI;
        this.isRayFacingUp = !this.isRayFacingDown;
        // angle is facing right if < 90 deg or greater than 270 deg (angle inncreases clockwise)
        this.isRayFacingRight = rayAngle < 0.5 * Math.PI || rayAngle > 1.5 * Math.PI;
        this.isRayFacingLeft = !this.isRayFacingRight;
        this.wasHitVertical = false;
    }
    cast(columnnId) {
        var xstep, ystep;
        var xintercept, yintercept;

        ////////////////////////////////////////////////////////
        // HORIZONTAL RAY-GRID INTERSECTIONS
        ////////////////////////////////////////////////////////
        var horzWallHitX = null;
        var horzWallHitY = null;
        var foundHorzWallHit = false;

        // Calculate the Y coordinate of the closest horizontal grid intersection
        yintercept = Math.floor(player.y / TILE_SIZE) * TILE_SIZE;
        yintercept += this.isRayFacingDown ? TILE_SIZE : 0;

        xintercept = player.x + (yintercept - player.y) / Math.tan(this.rayAngle);

        // Calculate the increment xstep and ystep
        ystep = TILE_SIZE;
        ystep *= this.isRayFacingUp ? -1 : 1;

        xstep = TILE_SIZE / Math.tan(this.rayAngle);
        xstep *= (this.isRayFacingLeft && xstep > 0) ? -1 : 1;
        xstep *= (this.isRayFacingRight && xstep < 0) ? -1 : 1;

        var nextHorzTouchX = xintercept;
        var nextHorzTouchY = yintercept;

        if (this.isRayFacingUp)
            nextHorzTouchY--;

        // Increment xstep and ystep until finds a wall
        while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= WINDOW_HEIGHT) {
            if (grid.hasWallAt(nextHorzTouchX, nextHorzTouchY)) {
                horzWallHitX = nextHorzTouchX;
                horzWallHitY = nextHorzTouchY;
                foundHorzWallHit = true;
                break;
            } else {
                nextHorzTouchX += xstep;
                nextHorzTouchY += ystep;
            }
        }

        ////////////////////////////////////////////////////////
        // VERTICAL RAY-GRID INTERSECTIONS
        ////////////////////////////////////////////////////////
        var vertWallHitX = 0;
        var vertWallHitY = 0;
        var foundVertWallHit = false;

        // Calculate the Y coordinate of the closest vertical grid intersection
        xintercept = Math.floor(player.x / TILE_SIZE) * TILE_SIZE;
        xintercept += this.isRayFacingRight ? TILE_SIZE : 0;

        yintercept = player.y + (xintercept - player.x) * Math.tan(this.rayAngle);

        // Calculate the increment xstep and ystep
        xstep = TILE_SIZE;
        xstep *= this.isRayFacingLeft ? -1 : 1;

        ystep = TILE_SIZE * Math.tan(this.rayAngle);
        ystep *= (this.isRayFacingUp && ystep > 0) ? -1 : 1;
        ystep *= (this.isRayFacingDown && ystep < 0) ? -1 : 1;

        var nextVertTouchX = xintercept;
        var nextVertTouchY = yintercept;

        if (this.isRayFacingLeft)
            nextVertTouchX--;

        // increment xstep and ystep until finds a wall
        while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 && nextVertTouchY < WINDOW_HEIGHT) {
            if (grid.hasWallAt(nextVertTouchX, nextVertTouchY)) {
                vertWallHitX = nextVertTouchX;
                vertWallHitY = nextVertTouchY;
                foundVertWallHit = true;
                break;
            } else {
                nextVertTouchX += xstep;
                nextVertTouchY += ystep;
            }
        }

        // Calculate both horizontal and vertical hit distances and choose the closest one
        var horzHitDistance =
            (foundHorzWallHit)
                ? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
                : Number.MAX_VALUE;
        var vertHitDistance =
            (foundVertWallHit)
                ? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
                : Number.MAX_VALUE;

        this.wallHitX = (vertHitDistance < horzHitDistance) ? vertWallHitX : horzWallHitX;
        this.wallHitY = (vertHitDistance < horzHitDistance) ? vertWallHitY : horzWallHitY;
        this.wasHitVertical = (vertHitDistance < horzHitDistance);
        this.distance = (vertHitDistance < horzHitDistance) ? vertHitDistance : horzHitDistance;
    }
    draw() {
        stroke('rgba(255, 0, 0, 0.2)');
        line(
            MINIMAP_SCALE_FACTOR * player.x,
            MINIMAP_SCALE_FACTOR * player.y,
            MINIMAP_SCALE_FACTOR * this.wallHitX,
            MINIMAP_SCALE_FACTOR * this.wallHitY
        );
    }
}

////////////////////////////////////////////////////////////////
// Global objects and variables
////////////////////////////////////////////////////////////////
var grid = new Grid();
var player = new Player();
var rays = [];

////////////////////////////////////////////////////////////////
// Function for key pressed event
////////////////////////////////////////////////////////////////
function keyPressed() {
    if (keyCode == UP_ARROW) {
        player.walkDirection = 1;
    } else if (keyCode == DOWN_ARROW) {
        player.walkDirection = -1;
    } else if (keyCode == LEFT_ARROW) {
        player.turnDirection = -1;
    } else if (keyCode == RIGHT_ARROW) {
        player.turnDirection = 1;
    } else if (keyCode == 77) {
        renderOption = 1;
    } else if (keyCode == 87) {
        renderOption = 2;
    }
}

////////////////////////////////////////////////////////////////
// Function for key released event
////////////////////////////////////////////////////////////////
function keyReleased() {
    if (keyCode == UP_ARROW || keyCode == DOWN_ARROW) {
        player.walkDirection = 0;
    } else if (keyCode == LEFT_ARROW || keyCode == RIGHT_ARROW) {
        player.turnDirection = 0;
    }
}

////////////////////////////////////////////////////////////////
// Cast rays
////////////////////////////////////////////////////////////////
function castAllRays() {
    var columnnId = 0;

    // Start first ray subtracting half of the FOV
    var rayAngle = player.rotationAngle - (FOV_ANGLE / 2);

    // clear the array of rays
    rays = [];

    // Loop all columns casting rays
    for (var i = 0; i < NUM_RAYS; i++) {
        // call function to cast a single ray
        var ray = new Ray(rayAngle);
        ray.cast(columnnId);
        rays.push(ray);

        // next ray is last ray incremented by FOV_ANGLE / NUM_RAYS
        rayAngle += FOV_ANGLE / NUM_RAYS;

        columnnId++;
    }
}

////////////////////////////////////////////////////////////////
// Function to draw wall rectangle strips for each ray
////////////////////////////////////////////////////////////////
function render3DProjection(angle) {
    // loop every ray in the array
    for (var i = 0; i < NUM_RAYS; i++) {
        // get the perpendicular distance
        var rayDistance = rays[i].distance;// * cos(rays[i].rayAngle - player.rotationAngle);
        var distanceToProjectionPlane = (WINDOW_WIDTH / 2) / Math.tan(FOV_ANGLE / 2);
        var projectedWallHeight = (TILE_SIZE / rayDistance * distanceToProjectionPlane);
        var alpha = (150 / rayDistance);
        fill("rgba(255, 255, 255, " + alpha + ")");
        noStroke();
        rect(
            i * WALL_STRIP_WIDTH,
            WINDOW_HEIGHT / 2 - projectedWallHeight / 2,
            30.0,
            projectedWallHeight
        );
    }
}

////////////////////////////////////////////////////////////////
// Function to maintain angle between 0 and 360 deg
////////////////////////////////////////////////////////////////
function normalizeAngle(angle) {
    angle = angle % (2 * Math.PI);
    if (angle < 0) {
        angle = (2 * Math.PI) + angle;
    }
    return angle;
}

////////////////////////////////////////////////////////////////
// Returns the distance between two points
////////////////////////////////////////////////////////////////
function distanceBetweenPoints(x1, y1, x2, y2) {
    return Math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

////////////////////////////////////////////////////////////////
// Function to initialize values and objects
////////////////////////////////////////////////////////////////
function setup() {
    createCanvas(WINDOW_WIDTH, WINDOW_HEIGHT);
}

////////////////////////////////////////////////////////////////
// Function to update objects and variables per frame
////////////////////////////////////////////////////////////////
function update() {
    player.update();
    castAllRays();
}

////////////////////////////////////////////////////////////////
// Function to render objects per frame
////////////////////////////////////////////////////////////////
function draw() {
    update();
    clear(31);

    render3DProjection();

    // grid.draw();
    // for (ray of rays) {
    //     ray.draw();
    // }
    // player.draw();
}
