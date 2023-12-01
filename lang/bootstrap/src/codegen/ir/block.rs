use std::collections::HashMap;

use super::{IRState, IRWalkable, Scope};
use crate::{ast::block::Block, syntax_error::SyntaxError};

impl IRWalkable for Block {
    type Output = ();

    fn walk_ir<'a>(&'a self, ir: &mut IRState<'a>) -> Result<Self::Output, SyntaxError> {
        let old_scope = ir.scope.clone();

        ir.scope = match &ir.scope {
            None => Some(Scope {
                previous_scope: None,
                symbols: HashMap::new(),
            }),
            Some(scope) => Some(Scope {
                previous_scope: Some(Box::new(scope.clone())),
                symbols: HashMap::new(),
            }),
        };

        for statement in &self.statements {
            statement.walk_ir(ir)?;
        }

        ir.scope = old_scope;

        Ok(())
    }
}
