#include "first_app.hpp"
#include <stdexcept>

namespace lve {

FirstApp::FirstApp() {
    loadModels();
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
    // start = time(0);
    // iterations = 0;
}

FirstApp::~FirstApp() {
    vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

std::vector<LveModel::Vertex> sierpinskiTriangle(std::vector<LveModel::Vertex> vertices) {
    std::vector<LveModel::Vertex> newVertices;
    LveModel::Vertex vertex;
    for(int i = 0; i < vertices.size() / 3; i++) {

        // First sierpinski subsection
        newVertices.push_back(vertices[i * 3]);
        vertex.position = (vertices[i * 3].position + vertices[i * 3 + 1].position)/glm::vec2(2.0f, 2.0f);
        newVertices.push_back(vertex);
        vertex.position = (vertices[i * 3].position + vertices[i * 3 + 2].position)/glm::vec2(2.0f, 2.0f);
        newVertices.push_back(vertex);

        // Second sierpinski subsection
        vertex.position = (vertices[i * 3 + 1].position + vertices[i * 3].position)/glm::vec2(2.0f, 2.0f);
        newVertices.push_back(vertex);
        newVertices.push_back(vertices[i * 3 + 1]);
        vertex.position = (vertices[i * 3 + 1].position + vertices[i * 3 + 2].position)/glm::vec2(2.0f, 2.0f);
        newVertices.push_back(vertex);

        // Third sierpinski subsection
        vertex.position = (vertices[i * 3 + 2].position + vertices[i * 3].position)/glm::vec2(2.0f, 2.0f);
        newVertices.push_back(vertex);
        vertex.position = (vertices[i * 3 + 2].position + vertices[i * 3 + 1].position)/glm::vec2(2.0f, 2.0f);
        newVertices.push_back(vertex);
        newVertices.push_back(vertices[i * 3 + 2]);
    }
    return newVertices;
}

void FirstApp::run() {
    while(!lveWindow.shouldClose()) {
        glfwPollEvents();
        // if((int)difftime( time(0), start) != iterations && iterations < 10) {
        //     vkDeviceWaitIdle(lveDevice.device());
        //     vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
        //     iterations++;
        //     loadModels();
        //     createPipelineLayout();
        //     createPipeline();
        //     createCommandBuffers();
        // }
        drawFrame();
    }

    vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadModels() {
    std::vector<LveModel::Vertex> vertices {{{0.0f, -0.9f}}, {{0.9f, 0.9f}}, {{-0.9f, 0.9f}}};
    // for(int i = 0; i < iterations; i++){
    //     vertices  = sierpinskiTriangle(vertices);
    // }
    lveModel = std::make_unique<LveModel>(lveDevice, vertices);
}

void FirstApp::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if(vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void FirstApp::createPipeline() {
    PipelineConfigInfo pipelineConfig{};
    LvePipeline::defaultPipelineConfigInfo(pipelineConfig, lveSwapChain.width(), lveSwapChain.height());
    pipelineConfig.renderPass = lveSwapChain.getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;
    lvePipeline = std::make_unique<LvePipeline>(
        lveDevice,
        "shaders/simple_shader.vert.spv",
        "shaders/simple_shader.frag.spv",
        pipelineConfig);
}

void FirstApp::createCommandBuffers(){
    commandBuffers.resize(lveSwapChain.imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = lveDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if(vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for(int i = 0; i < commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        
        if(vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = lveSwapChain.getRenderPass();
        renderPassInfo.framebuffer = lveSwapChain.getFrameBuffer(i);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = lveSwapChain.getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, static_cast<uint32_t>(0.0f)};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        lvePipeline->bind(commandBuffers[i]);
        lveModel->bind(commandBuffers[i]);
        lveModel->draw(commandBuffers[i]);

        vkCmdEndRenderPass(commandBuffers[i]);
        if(vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}
void FirstApp::drawFrame() {
    uint32_t imageIndex;
    auto result = lveSwapChain.acquireNextImage(&imageIndex);

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    result = lveSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

}