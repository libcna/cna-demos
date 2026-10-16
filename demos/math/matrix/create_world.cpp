// SPDX-License-Identifier: MS-PL
// Matrix::CreateWorld and the direction properties of a matrix.
#include <cmath>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"

using namespace Microsoft::Xna::Framework;

namespace
{
    bool IsNaN(const Vector3& value)
    {
        return std::isnan(value.X) && std::isnan(value.Y) && std::isnan(value.Z);
    }

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

        // The signed direction pairs share three matrix rows. Setting either spelling must make
        // its opposite getter return the exact negation, just as XNA's mutable properties do.
        Matrix authoredAxes = Matrix::getIdentityProperty();
        authoredAxes.setRightProperty(Vector3(2.0f, 3.0f, 4.0f));
        const bool rightSet = authoredAxes.getRightProperty() == Vector3(2.0f, 3.0f, 4.0f) &&
                              authoredAxes.getLeftProperty() == Vector3(-2.0f, -3.0f, -4.0f);
        authoredAxes.setLeftProperty(Vector3(-5.0f, 6.0f, -7.0f));
        ctx.Check(rightSet && authoredAxes.getLeftProperty() == Vector3(-5.0f, 6.0f, -7.0f) &&
                      authoredAxes.getRightProperty() == Vector3(5.0f, -6.0f, 7.0f),
                  "Right and Left setters write opposite views of the first row");

        authoredAxes.setUpProperty(Vector3(8.0f, 9.0f, 10.0f));
        const bool upSet = authoredAxes.getUpProperty() == Vector3(8.0f, 9.0f, 10.0f) &&
                           authoredAxes.getDownProperty() == Vector3(-8.0f, -9.0f, -10.0f);
        authoredAxes.setDownProperty(Vector3(-11.0f, 12.0f, -13.0f));
        ctx.Check(upSet && authoredAxes.getDownProperty() == Vector3(-11.0f, 12.0f, -13.0f) &&
                      authoredAxes.getUpProperty() == Vector3(11.0f, -12.0f, 13.0f),
                  "Up and Down setters write opposite views of the second row");

        authoredAxes.setBackwardProperty(Vector3(14.0f, 15.0f, 16.0f));
        const bool backwardSet = authoredAxes.getBackwardProperty() == Vector3(14.0f, 15.0f, 16.0f) &&
                                 authoredAxes.getForwardProperty() == Vector3(-14.0f, -15.0f, -16.0f);
        authoredAxes.setForwardProperty(Vector3(-17.0f, 18.0f, -19.0f));
        ctx.Check(backwardSet && authoredAxes.getForwardProperty() == Vector3(-17.0f, 18.0f, -19.0f) &&
                      authoredAxes.getBackwardProperty() == Vector3(17.0f, -18.0f, 19.0f),
                  "Backward and Forward setters write opposite views of the third row");

        // XNA normalizes the three derived axes without repairing degenerate input. Position and
        // affine constants remain valid even when one or all orientation rows become NaN.
        const Matrix zeroForward = Matrix::CreateWorld(position, Vector3::Zero, Vector3::Up);
        ctx.Check(IsNaN(zeroForward.getRightProperty()) && IsNaN(zeroForward.getUpProperty()) &&
                      IsNaN(zeroForward.getForwardProperty()) &&
                      zeroForward.getTranslationProperty() == position && zeroForward.M44 == 1.0f,
                  "a zero forward vector publishes three NaN axes but preserves position");

        const Matrix zeroUp = Matrix::CreateWorld(position, Vector3::Forward, Vector3::Zero);
        ctx.Check(IsNaN(zeroUp.getRightProperty()) && IsNaN(zeroUp.getUpProperty()) &&
                      zeroUp.getForwardProperty() == Vector3::Forward && zeroUp.getTranslationProperty() == position,
                  "a zero up vector preserves forward while lateral axes become NaN");

        const Matrix parallel = Matrix::CreateWorld(position, Vector3::Up, Vector3::Up);
        ctx.Check(IsNaN(parallel.getRightProperty()) && IsNaN(parallel.getUpProperty()) &&
                      parallel.getForwardProperty() == Vector3::Up && parallel.getTranslationProperty() == position,
                  "parallel forward and up preserve forward while lateral axes become NaN");

        const Vector3 scaledForward(2.0f, 0.0f, 0.0f);
        const Vector3 scaledUp(0.0f, 3.0f, 0.0f);
        const Matrix scaledDirections = Matrix::CreateWorld(position, scaledForward, scaledUp);
        ctx.Check(scaledDirections.getRightProperty() == Vector3::Backward &&
                      scaledDirections.getUpProperty() == Vector3::Up &&
                      scaledDirections.getForwardProperty() == Vector3::Right,
                  "nonunit forward and up vectors produce a normalized orthonormal basis");

        Matrix byReference;
        Matrix::CreateWorld(position, scaledForward, scaledUp, byReference);
        ctx.Check(byReference == scaledDirections, "the by-reference overload matches the normalized value result");
    }
}

CNA_DEMOS_CONSOLE(MatrixCreateWorld, {
    .id = "math.matrix.create_world",
    .title = "World matrix from position and facing",
    .description = "Builds world matrices from normalized, scaled and degenerate directions, and exercises every "
                   "mutable signed-axis property plus translation.",
    .apis = {"Matrix::CreateWorld", "Matrix::Forward", "Matrix::Backward", "Matrix::Up", "Matrix::Down",
             "Matrix::Left", "Matrix::Right", "Matrix::Translation"},
    .tags = {"matrix", "orientation", "axes"},
})
