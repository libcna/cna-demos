// SPDX-License-Identifier: MS-PL
// Matrix::CreateWorld and the direction properties of a matrix.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    void MatrixCreateWorld(CnaDemos::DemoContext& ctx)
    {
        // An object at (3, 0, -5) facing +X, upright.
        const Vector3 position(3.0f, 0.0f, -5.0f);
        const Matrix world = Matrix::CreateWorld(position, Vector3::Right, Vector3::Up);

        // A matrix's rows are its local axes; the properties read them back.
        ctx.Print("Forward     = {}", world.getForwardProperty().ToString());
        ctx.Print("Up          = {}", world.getUpProperty().ToString());
        ctx.Print("Right       = {}", world.getRightProperty().ToString());
        ctx.Print("Translation = {}", world.getTranslationProperty().ToString());

        ctx.Check(world.getTranslationProperty() == position, "Translation is the object's position");
        ctx.CheckNear(Vector3::Distance(world.getForwardProperty(), Vector3::Right), 0.0, 1e-6, "Forward is the requested facing");
        ctx.CheckNear(Vector3::Distance(world.getUpProperty(), Vector3::Up), 0.0, 1e-6, "Up stays up");
        ctx.CheckNear(Vector3::Distance(world.getRightProperty(), Vector3::Backward), 0.0, 1e-6,
                      "facing +X, the object's right-hand side is +Z");
        ctx.Check(world.getBackwardProperty() == -world.getForwardProperty()
                      && world.getLeftProperty() == -world.getRightProperty()
                      && world.getDownProperty() == -world.getUpProperty(),
                  "Backward, Left and Down are the negations");

        // Moving an object "forward" uses its own axis.
        Matrix moved = world;
        moved.setTranslationProperty(world.getTranslationProperty() + world.getForwardProperty() * 2.0f);
        ctx.Check(moved.getTranslationProperty() == Vector3(5.0f, 0.0f, -5.0f), "two units forward along its facing");
    }
}

CNA_DEMOS_CONSOLE(MatrixCreateWorld, {
    .id = "math.matrix.create_world",
    .title = "World matrix from position and facing",
    .description = "Builds an object's world matrix with Matrix::CreateWorld and reads its Forward, Up, Right "
                   "and Translation properties back.",
    .apis = {"Matrix::CreateWorld", "Matrix::Forward", "Matrix::Backward", "Matrix::Up", "Matrix::Down",
             "Matrix::Left", "Matrix::Right", "Matrix::Translation"},
    .tags = {"matrix", "orientation", "axes"},
})
