//
// Created by shiyin on 2023/12/8.
//

#include "../core/bvh.h"

BVH::BVH(std::vector<Model*> models, int maxPrims, SplitMethod method) : maxPrimsInNode(maxPrims), splitMethod(method){
    root = nullptr;
    nodeNumber = 0;

    time_t start, end;
    time(&start);
    if (models.empty())return;
    root = build(std::move(models));
    time(&end);
    double seconds = difftime(end, start);

    Logger::Log<BVH>(__FUNCTION__ ,"BVH Generation complete : Time taken " + std::to_string(seconds) + " seconds.");
    Logger::Log<BVH>(__FUNCTION__ ,"number of node is : " + std::to_string(nodeNumber));
    Logger::Log<BVH>(__FUNCTION__ ,"---------------------BVH GENERATION--------------------");

}

BVHNode *BVH::build(std::vector<Model*> models) {
    auto* node = new BVHNode();
    if (models.size() == 1){
        node = new BVHNode();
        node->box = models[0]->box;
        node->modelPtr = models[0];
        node->left = nullptr;
        node->right = nullptr;
        nodeNumber++;
        return node;
    } else if (models.size() == 2){
        node->left = build(std::vector<Model*>{models[0]});
        node->right = build(std::vector<Model*>{models[1]});
        node->box = Union(node->left->box,node->right->box);
        return node;
    }else{
        BoundingBox3 centroidBounds;
        for (auto model : models) {
            centroidBounds = Union(centroidBounds,model->box.Centroid());
        }
        int dim = centroidBounds.maxExtent();
        switch (dim) {
            case 0:
                std::sort(models.begin(),models.end(),[](Model* m1,Model* m2){
                    return m1->box.Centroid().x < m2->box.Centroid().x;
                });
                break;
            case 1:
                std::sort(models.begin(),models.end(),[](Model* m1,Model* m2){
                    return m1->box.Centroid().y < m2->box.Centroid().y;
                });
                break;
            case 2:
                std::sort(models.begin(),models.end(),[](Model* m1,Model* m2){
                    return m1->box.Centroid().z < m2->box.Centroid().z;
                });
                break;
            default:
                break;
        }

        auto beginning = models.begin();
        auto middling = models.begin() + (models.size() / 2);
        auto ending = models.end();

        auto leftModels = std::vector<Model*>(beginning, middling);
        auto rightModels = std::vector<Model*>(middling, ending);

        assert(models.size() == (leftModels.size() + rightModels.size()));

        node->left = build(leftModels);
        node->right = build(rightModels);

        node->box = Union(node->left->box, node->right->box);
    }
    return node;
}

BVH::~BVH() {
    if (root){
        delete root;
        root = nullptr;
    }
}

int BVH::getNodeNumber() const {
    return nodeNumber;
}

BVHNode *BVH::getRootNode() const {
    return root;
}

Intersection BVH::intersect(const Ray &ray) {
    Intersection intersection;
    if (!root) return intersection;
    intersection = getIntersection(root,ray);
    return intersection;
}

Intersection BVH::getIntersection(BVHNode *node, const Ray &ray) {
    Intersection intersection;

    // 节点为空或者与当前包围盒无交点直接返回false
    if (node == nullptr || !node->box.intersect(ray)) return intersection;

    // 只有一个节点
    if (node->left == nullptr && node->right == nullptr){
        intersection = node->modelPtr->intersect(ray);
        return intersection;
    }

    // 遍历左子树和右子树
    Intersection hitLeft, hitRight;
    if (node->left) hitLeft = getIntersection(node->left,ray);
    if (node->right) hitRight = getIntersection(node->right,ray);

    // 返回较近的一个交点
    return hitLeft.distance < hitRight.distance ? hitLeft : hitRight;
}

std::vector<BoundingBox3> BVH::getBVHBoundingBox() {
    std::vector<BoundingBox3> bvhBoxes;
    traverseBvhBox(root, bvhBoxes);

    return bvhBoxes;
}

void BVH::traverseBvhBox(BVHNode *node, std::vector<BoundingBox3>& boxes) {
    if (node == nullptr) return;

    boxes.push_back(node->box);

    traverseBvhBox(node->left,boxes);
    traverseBvhBox(node->right,boxes);
}
