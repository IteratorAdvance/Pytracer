#ifndef MESH_H
#define MESH_H

#include "geometry.h"
#include "kdtree.h"
#include "compress.h"


inline pair<vector<Vector>, vector<vector<int> > > read_rabbit_as_vertices(char *filename, int &n, int &m) {
    FILE *file = fopen(filename, "r");
    fscanf(file, "%d%d", &n, &m);
    vector<Vector> vertices;
    for (int i = 0; i < n; i++) {
        real x, y, z;
        char _[1000], __[1000];
        fscanf(file, "%s%lf%lf%lf", _, &x, &y, &z);
        vertices.push_back(Vector(x, y, z, 1));
    }

    vector<vector<int> > triangles;

    for (int i = 0; i < m; i++) {
        int vec_id[3]; char _[1000];
        fscanf(file, "%s%d%d%d", _, &vec_id[0], &vec_id[1], &vec_id[2]);
        for (int j = 0; j < 3; j++) vec_id[j] -= 1;
        vector<int> tmp(vec_id, vec_id + 3);
//        cout << tmp << endl;
        triangles.push_back(tmp);
    }
    return make_pair(vertices, triangles);
}

inline vector<vector<Point> > combine_vertices(pair<vector< vector<Vector> >, vector<vector<int> > > rabbit, int n, int m);

inline vector<vector<Point> > recieve_obj(int nV, int nVt, int nVn, int nF, double compress, double *values, int *vertices) {

    pair<vector< vector<Vector> >, vector<vector<int> > > result;
    vector<Vector> V, Vt, Vn;
    for (int i = 0; i < nV; i++) {
        int index = (i) * 4;
        V.push_back(Vector(values[index], values[index + 1], values[index + 2], values[index + 3]));
    }
    for (int i = 0; i < nVt; i++) {
        int index = (nV + i) * 4;
        Vt.push_back(Vector(values[index], values[index + 1], values[index + 2], values[index + 3]));
    }
    for (int i = 0; i < nVn; i++) {
        int index = (nV + nVt + i) * 4;
        Vn.push_back(Vector(values[index], values[index + 1], values[index + 2], values[index + 3]));
    }

    result.first.resize(3);
    result.first[0] = V;
    result.first[1] = Vt;
    result.first[2] = Vn;


    for (int i = 0; i < nF; i++) {
        vector<int> face;
        int index = i * 9;
        for (int j = 0; j < 3; j++)
            face.push_back(vertices[index + j * 3]);
        for (int j = 0; j < 3; j++)
            face.push_back(vertices[index + j * 3 + 1]);
        if (vertices[index + 2] != -1 && vertices[index + 5] != -1 && vertices[index + 8] != -1)
            for (int j = 0; j < 3; j++)
                face.push_back(vertices[index + j * 3 + 2]);
        result.second.push_back(face);
    }

       auto simplified = simplify(result.first[0], result.second, compress);
       result.first[0] = simplified.first;
       result.second = simplified.second;
//    for (int i = 0; i < nV; i++) {
//        printf("%f %f %f %f\n", V[i][0], V[i][1], V[i][2], V[i][3]);
//    }
//
//    for (int i = 0; i < nF; i++) {
//        printf("%d %d %d\n", result.second[i][0], result.second[i][1], result.second[i][2]);
//    }


    return combine_vertices(result, (int)result.first[0].size(), (int)result.second.size());
}




inline vector<vector<Point> > combine_vertices(pair<vector<vector<Vector> >, vector<vector<int> > > rabbit, int n, int m) {
    vector<Vector> V = rabbit.first[0];
    vector<Vector> Vt = rabbit.first[1];
    vector<Vector> Vn = rabbit.first[2];
    vector<vector<Vector> > triangles;
    vector<Vector> normals;
//    vector<real> areas;
    normals.resize((unsigned)n);
    //DAMN!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    for (int i = 0; i < n; i++) normals[i] = Vector(0, 0, 0, 0);
//    areas.resize((unsigned)n);
    for (auto item : rabbit.second) {
        vector<Vector> triangle;
        for (int i = 0; i < 3; i++) {
            Vector vec = V[item[i]];
            triangle.push_back(vec);
        }
        for (int i = 0; i < 3; i++) {
            if (item[i + 3] != -1) {
                Vector vec = Vt[item[i + 3]];
                triangle.push_back(vec);
            } else {
                triangle.push_back(Vector(0, 0, 0, 0));
            }
        }
        triangles.push_back(triangle);

        //GEt notmalk deleted
        Vector normal = GetNormal(cross((triangle[1] - triangle[0]),(triangle[2] - triangle[0])));
        for (int i = 0; i < 3; i++) {
            real dot = GetNormal(triangle[(i + 1) % 3] - triangle[i]).dot(GetNormal(triangle[(i + 2) % 3] - triangle[i]));
//            real area = acos(dot);
            real area = length(cross((triangle[1] - triangle[0]),(triangle[2] - triangle[0])));
            Vector tmp = normal * area;
//            cout << tmp << endl << endl;
//            assert(sgn(tmp[3]) == 0);

//            printf("dot %lf ares %lf\n", dot, area);
            normals[item[i]] += tmp;
//            areas[item[i]] += area;
        }
    }
    for (int i = 0; i < n; i++) {
//        cout << normals[i] << endl << endl;
        normals[i] = GetNormal(normals[i]);
    }
    for (int j = 0; j < m; j++) {
        vector<int> item = rabbit.second[j];
        for (int i = 0; i < 3; i++) {
//            cout << normals[item[i]] << endl << endl;
            triangles[j].push_back(normals[item[i]]);
        }
    }
    return triangles;
}


//inline vector<vector<Point> > read_rabbit_as_triangles(char *filename) {
//    int n, m;
//    pair<vector<Vector>, vector<vector<int> > > rabbit = read_rabbit_as_vertices(filename, n, m);
//    return combine_vertices(rabbit, n, m);
//}


class Mesh : public Shape {
public:
    vector<Triangle *> triangles;
    Vector center;
    real radius;
    Filter filter;
    KDTree *kdTree;
    Texture *texture;

    Mesh(vector<vector<Vector> > vector_triangles, vector<int> texture_id, vector<Texture *> textures){
//        if (strlen(texture_name) > 0) {
//            texture = new Texture(texture_name);
//        } else {
//            texture = NULL;
//        }
//        matrix = (Translation(toVector3(center)) * Eigen::Scaling(radius)).matrix();
        vector<vector<Point> > tmp_triangles = vector_triangles;

        int count = 0;
        for (auto triangle: tmp_triangles) {
            int texture = texture_id[count];
            Triangle *tri;
            if (texture != -1) {
//                assert(false);
//                cout << texture << endl;
                tri = new Triangle(triangle, textures[texture]);
            }
            else
                tri = new Triangle(triangle, NULL);
            this->triangles.push_back(tri);
            count += 1;
        }
        kdTree = new KDTree(triangles);
    }


//    Mesh(char *filename, Point center, real radius){
//        Matrix matrix;
////        matrix = (Translation(toVector3(center)) * Eigen::Scaling(radius)).matrix();
//        matrix = (Translation(toVector3(center)) * Eigen::Scaling(radius)).matrix();
//        SetMatrix(matrix);
//        vector<vector<Point> > tmp_triangles = read_rabbit_as_triangles(filename);
//
//        for (auto triangle: tmp_triangles) {
//            Triangle *tri = new Triangle(triangle, NULL);
//            this->triangles.push_back(tri);
//        }
//        kdTree = new KDTree(triangles);
//    }

    IntersectionInfo GetIntersectionInfo(const Ray &ray) {
        IntersectionInfo result = kdTree->TestIntersection(ray);
        return result;
    }

    Vector GetNormalAt(const Point &p) {
        return Vector(0, 0, 0, 0);
    }
    real GetIntersection(const Ray &line) {
        printf("invalid\n");
        return 0;
    }
    // real GetIntersection(Ray line) {
        // Vector dist = toVector(line.p);
        // Vector tmp = 2 * line.v;
        // real A = line.v.dot(line.v), B = (tmp).dot(dist), C = dist.dot(dist) - 1;
        // assert(false);
        // return 0;
    // }
    bool TestInside(const Point &point) {
        return length(point) < 1.0;
    }
};


#endif
