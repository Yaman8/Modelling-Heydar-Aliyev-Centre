#pragma once

#include<fstream>
#include<sstream>
#include<algorithm>
#include<vector>
#include "transform.h"
#include"Coord.h"
#include"camera.h"
#include"triangleRaster.h"
#include<array>
#include<string>

class Model {
private:
    std::vector<Triangle> triangles;
    std::vector<Triangle> ftriangles;
public:
    void load(std::string);
    void newLoad(std::string);
    void convertToScreen_model();
    void rotate_model(float);
    void translate_model(vect4);
    void scale_model(float);
    void applyTransform(mat4f&);
    void Transform(mat4f&, mat4f&);
    bool backfaceDetectionNormalize(Triangle& tri);
    void draw();
    float calcIntensity(vect4, vect4, vect4);
    void phongIlluminationModel(Triangle&);
    void diffuseLighting();
    void Shading(Triangle&);
    void loadTexture(std::string);
    bool backFaceCulling(Triangle& tri);
    bool Shade = true;

    Camera* camera;
};

void Model::loadTexture(std::string name) {

}

void Model::draw() {

    //drawWireframe_model(ftriangles);
    draw_model(ftriangles, Shade);
}

void Model::load(std::string filename)
{
    std::ifstream file;
    file.open(filename);
    if (file.fail())
    {
        std::cout << "File cannot be opened \n";
        exit(-1);
    }
    //std::cout << "open";
    // Local cache of verts
    std::vector<vect4> verts;

    while (!file.eof())
    {
        char line[128];
        file.getline(line, 128);

        std::stringstream s;
        s << line;

        char junk;

        if (line[0] == 'v')
        {
            vect4 v;
            s >> junk >> v.x >> v.y >> v.z;
            verts.push_back(v);
        }

        if (line[0] == 'f')
        {
            int f[3];
            s >> junk >> f[0] >> f[1] >> f[2];
            triangles.push_back(Triangle{ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
        }
    }
    ftriangles = triangles;
}

void Model::newLoad(std::string filename) {
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail())
    {
        std::cout << "Cannot Read" << std::endl;
        exit(-1);
    }
    std::string line;
    std::vector<vect4> verts;
    std::vector<vect4> normals;
    std::vector<vec2i> textures;

    int count = 1;
    while (!in.eof())
    {
        //get one line at a time
        std::getline(in, line);
        //string object
        std::istringstream iss(line.c_str());

        char trash;
        if (!line.compare(0, 2, "v ")) //starts with v<space>
        {

            //std::cout << count;
            iss >> trash; // first character is v
            vect4 v;
            // followed by xyz co-ords
            iss >> v.x;
            iss >> v.y;
            iss >> v.z;
            // maths::printvec(v);
            verts.push_back(v);
            count++;
        }
        else if (!line.compare(0, 3, "vt ")) //starts with vt<space>
        {
            iss >> trash >> trash; //Ignore vt
            vec2i uv;
            iss >> uv.x;
            iss >> uv.y;
            textures.push_back(uv);
        }

        else if (!line.compare(0, 3, "vn ")) //starts with vn<space>
        {
            iss >> trash >> trash;
            vect4 n;
            iss >> n.x;
            iss >> n.y;
            iss >> n.z;
            normals.push_back(n);
        }
        else if (!line.compare(0, 2, "f ")) //starts with f<space>
        {
            std::vector<vect4> f;
            vect4 temp;

            iss >> trash; //first charecter is f

            while (iss >> temp.x >> trash >> temp.y >> trash >> temp.z)
                // in the form vert/vertTex/norm (vert is read, the rest are treated as trash)
            {
                //in wavefront obj all indices start at 1, not zero
                temp.x--; //vert
                temp.y--; //texture
                temp.z--; // normal
                f.push_back(temp);
            }
            Triangle tri;
            tri.setVertex(verts[f[0].x], verts[f[1].x], verts[f[2].x]);
            // std::cout  << f[0][0] <<'\n';
            tri.setTexCoords(textures[f[0].y], textures[f[1].y], textures[f[2].y]);
            tri.setNormals(normals[f[0].z], normals[f[1].z], normals[f[2].z]);
            triangles.push_back(tri);
        }
    }
    for (size_t i = 0; i < triangles.size() - 1; i++)
    {
        triangles[i].zbuff = (triangles[i].vertices[0].z + triangles[i].vertices[1].z + triangles[i].vertices[2].z);
    }
    ftriangles = triangles;
}

void Model::convertToScreen_model() {
    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            triangles[i].vertices[j] = triangles[i].vertices[j].Convert_to_Screen();
        }
    }
}

void Model::translate_model(vect4 pt) {
    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            transate_polygon(triangles[i].vertices[j], pt);
        }
    }
}

void Model::scale_model(float pt) {
    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            scale_polygon(triangles[i].vertices[j], pt);
        }
    }
}

void Model::rotate_model(float angle) {
    for (int i = 0; i < triangles.size(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            //rotateX(triangles[i].vertices[j], angle);
            rotateY(triangles[i].vertices[j], angle);
            //rotateZ(triangles[i].vertices[j], angle);
        }
    }
}

void Model::Transform(mat4f& view, mat4f& projection) {
    ftriangles.clear();
    int cullCount = 0;

    for (auto& tri : triangles)
    {
        Triangle temptri = tri;

        // bool culled = backfaceDetectionNormalize(tri);
        bool culled = backFaceCulling(temptri);
        if (culled)
            continue;

        // view transformation
        temptri.vertices[0] = mul(view, temptri.vertices[0]);
        temptri.vertices[1] = mul(view, temptri.vertices[1]);
        temptri.vertices[2] = mul(view, temptri.vertices[2]);

        //Projection Transformation
        temptri.vertices[0] = mul(projection, temptri.vertices[0]);
        temptri.vertices[1] = mul(projection, temptri.vertices[1]);
        temptri.vertices[2] = mul(projection, temptri.vertices[2]);

        ftriangles.push_back(temptri);
    }


    //painterSort(ftriangles, 0, ftriangles.size());
    sort(ftriangles.begin(), ftriangles.end(), [](Triangle& t1, Triangle& t2)
        {
            float z1 = (t1.vertices[0].z + t1.vertices[1].z + t1.vertices[2].z) / 3.0f;
            float z2 = (t2.vertices[0].z + t2.vertices[1].z + t2.vertices[2].z) / 3.0f;
            return z1 > z2;
        });

    for (auto& tri : ftriangles)
    {
        // Point color = Point{220, 220, 220}.normalize();

        tri.color = vec3{ 0.8, 0.8, 0.8 };
        if (Shade)
            Shading(tri);
        else
            phongIlluminationModel(tri);
    }
}

bool Model::backFaceCulling(Triangle& tri)
{
    vect4 v1 = tri.vertices[0];
    vect4 v2 = tri.vertices[1];
    vect4 v3 = tri.vertices[2];

    vect4 n1 = tri.normals[0];
    vect4 n2 = tri.normals[1];
    vect4 n3 = tri.normals[2];

    // Point centroid;
    // centroid[0] = (v1[0] + v2[0] + v3[0]) / 3;
    // centroid[1] = (v1[1] + v2[1] + v3[1]) / 3;
    // centroid[2] = (v1[2] + v2[2] + v3[2]) / 3;

    vect4 view1 = (camera->Position - v1).normalize();
    vect4 view2 = (camera->Position - v2).normalize();
    vect4 view3 = (camera->Position - v3).normalize();

    // Point normal =maths::getnormal(centroid,tri.vertices[1],tri.vertices[2]);

    float dotProduct1 = dotProduct(n1, view1);
    float dotProduct2 = dotProduct(n2, view2);
    float dotProduct3 = dotProduct(n3, view3);

    if (dotProduct1 < 0 || dotProduct2 < 0 || dotProduct3 < 0)
    {
        return false;
    }
    return true;
}

bool Model::backfaceDetectionNormalize(Triangle& tri)
{
    vect4 v1 = tri.vertices[0], v2 = tri.vertices[1], v3 = tri.vertices[2];
    vect4 centroid;
    centroid.x = (v1.x + v2.x + v3.x) / 3;
    centroid.y = (v1.y + v2.y + v3.y) / 3;
    centroid.z = (v1.z + v2.z + v3.z) / 3;

    vect4 V = (camera->Position - centroid).normalize();
    //vect4 V = (vect4{ 0,0,-1000 } - centroid).normalize();

    v2 = v2 - v1;
    v3 = v3 - v1;

    vect4 normal = v2.crossProduct(v3);
    normal = normal.normalize();
    // std::cout<<normal;

    float value = dotProduct(normal, V);
    //if (value < 0)
    //{
    //    ftriangles.push_back(tri);
    //    return false;
    //}
    ftriangles.push_back(tri);
    return true;
}

float Model::calcIntensity(vect4 point, vect4 Normal, vect4 View)
{
    float i = 0.0;
    vect4 position = { 500, 600, -200 };
    vect4 Ldir = (position - point).normalize();
    //std::cout << point.x << "\t" << point.y << "\t" << point.z << "\n";
    float ambientInt = 0.9;
    float pointInt = 0.5;

    float ambientConstant = 1;
    float diffuseConstant = 0.7;
    float specularConstant = 0.8;

    float ambientLight = ambientConstant * ambientInt;

    float diffuseLight = std::max(0.0f, diffuseConstant * pointInt * dotProduct(Normal, Ldir));

    // Point R = maths::sub(maths::mul(Normal, (2 * maths::dot(Normal, Ldir))), Ldir);
    vect4 R = ((Normal * (2 * dotProduct(Normal, Ldir))) - Ldir).normalize();
    float specularExp = 32;
    float specularLight = specularConstant * pointInt * pow(dotProduct(R, View), specularExp);

    float tmp = ambientLight + specularLight + diffuseLight;
    tmp = tmp > 1 ? 1 : tmp;
    return tmp;
}

void Model::phongIlluminationModel(Triangle& tri)
{
    vect4 v1 = tri.vertices[0];
    vect4 v2 = tri.vertices[1];
    vect4 v3 = tri.vertices[2];

    vect4 centroid;
    centroid.x = (v1.x + v2.x + v3.x) / 3;
    centroid.y = (v1.y + v2.y + v3.y) / 3;
    centroid.z = (v1.z + v2.z + v3.z) / 3;

    // std::cout << centroid[0] <<"\t";

    // Point view = (camera->Position - centroid).normalize();
    vect4 view = (camera->Position - centroid).normalize();

    // generating the normal vector of a triangle
    vect4 ver2 = centroid - v3;
    vect4 ver1 = centroid - v2;

    vect4 normal = (ver1.crossProduct(ver2)).normalize();

    float intensity = calcIntensity(centroid, normal, view);
    //std::cout << "The intensity: " << intensity << "\n";
    vec3 newColor = tri.color * intensity;

    tri.color = newColor;
    // triangle.print();
}

void diffuseLighting(Triangle& tri) {
    vect4 v1 = tri.vertices[0];
    vect4 v2 = tri.vertices[1];
    vect4 v3 = tri.vertices[2];

    vect4 ver1 = v1 - v2;
    vect4 ver2 = v1 - v3;

    vect4 normal = (v1.crossProduct(ver2)).normalize();
}

void Model::Shading(Triangle& tri)
{
    std::vector<float>intensity(3);
    int count = 0;
    for (auto& vertex : tri.vertices)
    {
        vect4 view = (vect4{ 0, 0, 10 } - vertex).normalize();
        intensity[count] = calcIntensity(vertex, tri.normals[count], view);
        count++;
    }
    tri.setIntensity(intensity);
}