use crate::{
    ast::expression::{UnaryOperation, UnaryOperationType},
    codegen::ir::{IRState, IRStatement, IRWalkable, Register},
    syntax_error::SyntaxError,
};

impl IRWalkable for UnaryOperation {
    type Output = u32;

    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError> {
        let expression_register = self.expression.walk_ir(ir)?;

        match self.operation_type {
            UnaryOperationType::Plus => Ok(expression_register),
            UnaryOperationType::Minus => {
                let tmp_register = ir.current_register + 1;
                let rd = tmp_register + 1;
                ir.current_register = rd;

                ir.statements.push(IRStatement::LoadImmediate {
                    rd: Register(tmp_register),
                    imm: 0,
                });

                ir.statements.push(IRStatement::Subtract {
                    rd: Register(rd),
                    rs1: Register(tmp_register),
                    rs2: Register(expression_register),
                });

                Ok(rd)
            }
            UnaryOperationType::BitwiseNot => {
                let tmp_register = ir.current_register + 1;
                let rd = tmp_register + 1;
                ir.current_register = rd;

                ir.statements.push(IRStatement::LoadImmediate {
                    rd: Register(tmp_register),
                    imm: 0xFFFFFFFFFFFFFFFF,
                });

                ir.statements.push(IRStatement::Xor {
                    rd: Register(rd),
                    rs1: Register(expression_register),
                    rs2: Register(tmp_register),
                });

                Ok(rd)
            }
            UnaryOperationType::LogicalNot => unimplemented!(),
        }
    }
}
