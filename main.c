#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include "readScene.c"
#include "buildFile.c"

double* buildColor(Object** objects, Object** lights, double* color, double* Ro, double* Rd, int depth){
    
    if (depth == 0) {
        return color;
    }
    
    // paint pixel based on type
    double closestT = INFINITY;
    Object* closestObject = NULL;
    
    // ========================================================================(for each object)
    for (int i=1; objects[i] != 0; i ++) {
        double t = 0;
        
        // find closest intersection based on objects
        switch(objects[i]->kind) {
            case 0:
                break;
            case 1:
                t = cylinderIntersection(Ro, Rd, objects[i]->position, objects[i]->radius);
                if (t > 0 && t < closestT){
                    closestT = t;
                    closestObject = objects[i];
                }
                break;
            case 2:
                t = sphereIntersection(Ro, Rd, objects[i]->position, objects[i]->radius);
                if (t > 0 && t < closestT){
                    closestT = t;
                    closestObject = objects[i];
                    
                }
                break;
            case 3:
                t = planeIntersection(Ro, Rd, objects[i]->position, objects[i]->normal);
                if (t > 0 && t < closestT){
                    closestT = t;
                    closestObject = objects[i];
                }
                break;
            case 4:
                break;
            default:
                fprintf(stderr, "Error: Invalid type number: %i", objects[i]->kind);
                exit(1);
                break;
        }
    }
    
    // ========================================================================(object detected)
    if (closestT < INFINITY){
        // discover lights
        
        // new origin
        double* Ron = malloc(sizeof(double)*3);
        Ron[0] = closestT * Rd[0] + Ro[0];
        Ron[1] = closestT * Rd[1] + Ro[1];
        Ron[2] = closestT * Rd[2] + Ro[2];
        
        // new normal
        double* normal = malloc(sizeof(double)*3);
        normal[0] = 0;
        normal[1] = 0;
        normal[2] = 0;
        
        switch(closestObject->kind){
            case 2: // sphere
                normal[0] = Ron[0] - closestObject->position[0];
                normal[1] = Ron[1] - closestObject->position[1];
                normal[2] = Ron[2] - closestObject->position[2];
                normalize(normal);
                break;
            case 3: // plane
                normal = closestObject->normal;
                normalize(normal);
                break;
            default:
                break;
        }
        // new reflection vector
        double* Rdm = malloc(sizeof(double)*3);
        Rdm[0] = Rd[0] - dot(Rd, normal) * 2 * normal[0];
        Rdm[1] = Rd[1] - dot(Rd, normal) * 2 * normal[1];
        Rdm[2] = Rd[2] - dot(Rd, normal) * 2 * normal[2];
        
        double* reflectColor = malloc(sizeof(double)*3);
        reflectColor[0] = 0; // ambient_color[0];
        reflectColor[1] = 0; // ambient_color[1];
        reflectColor[2] = 0; // ambient_color[2];
        
        // ====================================================================(for each light)
        for (int j = 0; lights[j] !=NULL; j++){
            
            
            
            // new light direction
            double* Rdn = malloc(sizeof(double)*3);
            Rdn[0] = lights[j]->position[0] - Ron[0];
            Rdn[1] = lights[j]->position[1] - Ron[1];
            Rdn[2] = lights[j]->position[2] - Ron[2];
            
            
            
            double closestT = INFINITY;
            Object* closestShadowObject = NULL;
            
            // ================================================================(roll through all object, check for shadow)
            for (int k = 0; objects[k] != NULL; k++){
                
                if (objects[k] == closestObject){
                    continue;
                }
                double t = 0;
                
                
                // object->intersect()
                switch(objects[k]->kind){
                    case 0:
                        break;
                    case 1:
                        t = cylinderIntersection(Ron, Rdn, objects[k]->position, objects[k]->radius);
                        if (t > 0 && t < closestT){
                            closestT = t;
                            
                        }
                        break;
                    case 2:
                        t = sphereIntersection(Ron, Rdn, objects[k]->position, objects[k]->radius);
                        if (t > 0 && t < closestT){
                            closestT = t;
                        }
                        break;
                    case 3:
                        t = planeIntersection(Ron, Rdn, objects[k]->position, objects[k]->normal);
                        if (t > 0 && t < closestT){
                            closestT = t;
                        }
                        break;
                    case 4:
                        break;
                    default:
                        fprintf(stderr, "Error: Invalid type number: %i", objects[k]->kind);
                        exit(1);
                        break;
                        
                }
                
                if (closestT < INFINITY && closestT < dist(Ron, lights[j]->position)){
                    closestShadowObject = objects[k];
                } else {
                    continue;
                }
            }
            
            // ================================================================(if no shadow)
            
            // N, L, R, V
            // shinyness
            double NS = 7;
            
            // N
            double* N = malloc(sizeof(double)*3);
            switch(closestObject->kind){
                case 2: // sphere
                    N[0] = Ron[0] - closestObject->position[0];
                    N[1] = Ron[1] - closestObject->position[1];
                    N[2] = Ron[2] - closestObject->position[2];
                    normalize(N);
                    break;
                case 3: // plane
                    N = closestObject->normal;
                    normalize(N);
                    break;
                default:
                    break;
            }
            
            // L
            double* L = malloc(sizeof(double)*3);
            L = Rdn; // light_position - Ron;
            normalize(L);
            
            // R = reflection of L
            double* R = malloc(sizeof(double)*3);
            
            R[0] = 2 * N[0] * dot(N, L) - L[0];
            R[1] = 2 * N[1] * dot(N, L) - L[1];
            R[2] = 2 * N[2] * dot(N, L) - L[2];
            
            // V = Rd;
            double* V = malloc(sizeof(double)*3);
            V[0] = -1 * Rd[0];
            V[1] = -1 * Rd[1];
            V[2] = -1 * Rd[2];
            
            // diffuse
            double* diffuse = malloc(sizeof(double)*3);
            if (dot(N, L) > 0){
                diffuse[0] = closestObject->diffuseColor[0] * lights[j]->color[0] * dot(N, L);
                diffuse[1] = closestObject->diffuseColor[1] * lights[j]->color[0] * dot(N, L);
                diffuse[2] = closestObject->diffuseColor[2] * lights[j]->color[0] * dot(N, L);
            } else {
                diffuse[0] = 0;
                diffuse[1] = 0;
                diffuse[2] = 0;
            }
            
            // specular
            double* specular = malloc(sizeof(double)*3);
            specular[0] = 0; // uses object's specular color
            specular[1] = 0;
            specular[2] = 0;
            if (dot(V, R) > 0 && dot(N, L) > 0){
                specular[0] = closestObject->specularColor[0] * lights[j]->color[0] * exponent(dot(R, V), NS); // uses object's specular color
                specular[1] = closestObject->specularColor[1] * lights[j]->color[1] * exponent(dot(R, V), NS);
                specular[2] = closestObject->specularColor[2] * lights[j]->color[2] * exponent(dot(R, V), NS);
            } else {
                specular[0] = 0; // uses object's specular color
                specular[1] = 0;
                specular[2] = 0;
                
            }
            if (lights[j]->direction[0] == 0 && lights[j]->direction[1] == 0 && lights[j]->direction[2] == 0){
                color[0] += frad(lights[j]->radialA2, lights[j]->radialA1, lights[j]->radialA0, dist(Ron, lights[j]->position)) * (diffuse[0] + specular[0]);
                color[1] += frad(lights[j]->radialA2, lights[j]->radialA1, lights[j]->radialA0, dist(Ron, lights[j]->position)) * (diffuse[1] + specular[1]);
                color[2] += frad(lights[j]->radialA2, lights[j]->radialA1, lights[j]->radialA0, dist(Ron, lights[j]->position)) * (diffuse[2] + specular[2]);
            } else {
                color[0] += frad(lights[j]->radialA2, lights[j]->radialA1, lights[j]->radialA0, dist(Ron, lights[j]->position)) * fang(lights[j]->theta, lights[j]->direction, Ron, lights[j]->angularA0) * (diffuse[0] + specular[0]);
                color[1] += frad(lights[j]->radialA2, lights[j]->radialA1, lights[j]->radialA0, dist(Ron, lights[j]->position)) * fang(lights[j]->theta, lights[j]->direction, Ron, lights[j]->angularA0) * (diffuse[1] + specular[1]);
                color[2] += frad(lights[j]->radialA2, lights[j]->radialA1, lights[j]->radialA0, dist(Ron, lights[j]->position)) * fang(lights[j]->theta, lights[j]->direction, Ron, lights[j]->angularA0) * (diffuse[2] + specular[2]);
            }
            
            if (closestObject->reflectivity == 0) {
                return color;
            } else {
                
                // pull reflection off of surface by just a bit (reduces specs)
                Ron[0] = Ron[0] + 0.01 * normal[0];
                Ron[1] = Ron[1] + 0.01 * normal[1];
                Ron[2] = Ron[2] + 0.01 * normal[2];
                
                // recursion call
                reflectColor = buildColor(objects, lights, reflectColor, Ron, Rdm, depth - 1);
                
                color[0] = ((1 - closestObject->reflectivity) * color[0]) + (closestObject->reflectivity * reflectColor[0]);
                color[1] = ((1 - closestObject->reflectivity) * color[1]) + (closestObject->reflectivity * reflectColor[1]);
                color[2] = ((1 - closestObject->reflectivity) * color[2]) + (closestObject->reflectivity * reflectColor[2]);
            }
                // ================================================================(if shadow)
            if (closestShadowObject != NULL){
                color[0] /= 3;
                color[1] /= 3;
                color[2] /= 3;
                
                
            }
            
            
        }
    }
    
    return color;
}

// build image buffer based on objects
unsigned char* buildBuffer(Object** objects, int M, int N){
    
    // build lights objects
    Object** lights = malloc(sizeof(Object)*128);
    int lightCount = 0;
    for (int i=1; objects[i] != 0; i ++) {
        switch(objects[i]->kind) {
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                lights[lightCount] = objects[i];
                lightCount++;
                break;
            default:
                break;
        }
    }
    if (lightCount == 0){
        fprintf(stderr, "Error: No lights were found in scene");
        exit(1);
    }
    
    // camera center
    double cx = 0;
    double cy = 0;
    
    // camera width and height
    double h = objects[0]->height;
    double w = objects[0]->width;
    
    // open output file && write header
    unsigned char* buffer = malloc(sizeof(char)*M*N*10);
    unsigned char* bufferNode = buffer;
    
    // space for single pixel
    double* Ro = malloc(sizeof(double)*3);
    double* Rd = malloc(sizeof(double)*3);
    
    // build scene
    double pixheight = h / M;
    double pixwidth = w / N;
    
    // ================================================================================(for each pixel)
    for (int y = M; y >0; y -= 1) {
        for (int x = 0; x < N; x += 1) {
            // space for single pixel
            Ro[0] = 0;
            Ro[1] = 0;
            Ro[2] = 0;
            
            // Rd = normalize(P - Ro)
            Rd[0] = cx - (w/2) + pixwidth * (x + 0.5);
            Rd[1] = cy - (h/2) + pixheight * (y + 0.5);
            Rd[2] = 1;
            normalize(Rd);
            
            
            
            // create color list
            double* color = malloc(sizeof(double)*3);
            color[0] = 0.1; // ambient_color[0];
            color[1] = 0.1; // ambient_color[1];
            color[2] = 0.1; // ambient_color[2];
            
            // build color for each level
            color = buildColor(objects, lights, color, Ro, Rd, 7);
            
            *bufferNode++ = (255 * clamp(color[0]));
            *bufferNode++ = (255 * clamp(color[1]));
            *bufferNode++ = (255 * clamp(color[2]));
        }
    }
    
    // end buffer
    *bufferNode = '\0';
    return buffer;
}


int main(int argc, char* argv[]) {
    
    // scene width and height
    int M = atoi(argv[1]);
    int N = atoi(argv[2]);
    
    // read json, and build objects
    Object** objects = readScene(argv[3]);
    
    // build header buffer
    char* header = buildHeader(objects, M, N);
    
    // build image buffer
    unsigned char* buffer = buildBuffer(objects, M, N);
    
    // dump buffer to file
    buildFile(header, buffer, argv[4], M, N);
    return 0;
}
