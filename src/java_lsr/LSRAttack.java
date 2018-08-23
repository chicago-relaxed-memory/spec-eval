public class LSRAttack {
  public static void main(String[] args) {
    new LSRAttack().attack();
  }

  public void attack() {
    int total = 0;
    for(int i = 0; i < 1000000; i++) {
      total += lsrattackA0(true);
      total += lsrattackA1(true);
      total += lsrattackA0_const(true);
      total += lsrattackA1_const(true);
      total += lsrattackB0(true);
      total += lsrattackB1(true);
      total += lsrattackC0(true);
      total += lsrattackC1(true);
    }
    System.out.println("Hello world " + total);
  }

  private static int SECRET0 = 0;
  private static int SECRET1 = 1;

  public static final boolean alwaysFalse_const = false;

  private static int x_static, y_static, z_static;
  private int x_inst, y_inst, z_inst;

  public static int lsrattackA0(boolean withPrivileges) {
    x_static = 1;
    if(withPrivileges) x_static = SECRET0;
    if(y_static > 0) { z_static = 0x12345; } else { z_static = 0x23456; }
    return x_static + y_static + z_static;
  }
  public static int lsrattackA1(boolean withPrivileges) {
    x_static = 1;
    if(withPrivileges) x_static = SECRET1;
    if(y_static > 0) { z_static = 0x12345; } else { z_static = 0x23456; }
    return x_static + y_static + z_static;
  }

  public static int lsrattackA0_const(boolean withPrivileges) {
    x_static = 1;
    if(alwaysFalse_const) x_static = SECRET0;
    if(y_static > 0) { z_static = 0x12345; } else { z_static = 0x23456; }
    return x_static + y_static + z_static;
  }
  public static int lsrattackA1_const(boolean withPrivileges) {
    x_static = 1;
    if(alwaysFalse_const) x_static = SECRET1;
    if(y_static > 0) { z_static = 0x12345; } else { z_static = 0x23456; }
    return x_static + y_static + z_static;
  }

  public int lsrattackB0(boolean withPrivileges) {
    x_inst = 1;
    if(withPrivileges) x_inst = SECRET0;
    if(y_inst > 0) { z_inst = 0x12345; } else { z_inst = 0x23456; }
    return x_inst + y_inst + z_inst;
  }
  public int lsrattackB1(boolean withPrivileges) {
    x_inst = 1;
    if(withPrivileges) x_inst = SECRET1;
    if(y_inst > 0) { z_inst = 0x12345; } else { z_inst = 0x23456; }
    return x_inst + y_inst + z_inst;
  }

  public int lsrattackC0(boolean withPrivileges) {
    if(withPrivileges) x_inst = SECRET0;
    else x_inst = 1;
    if(y_inst > 0) { z_inst = 0x12345; } else { z_inst = 0x23456; }
    return x_inst + y_inst + z_inst;
  }
  // HotSpot (trained on withPrivileges=TRUE):
  // if(!withPrivileges) bail
  // r = y_inst;
  // x_inst = SECRET0;
  // if(r > 0) bail
  // z_inst = 0x23456;
  // ...
  public int lsrattackC1(boolean withPrivileges) {
    if(withPrivileges) x_inst = SECRET1;
    else x_inst = 1;
    if(y_inst > 0) { z_inst = 0x12345; } else { z_inst = 0x23456; }
    return x_inst + y_inst + z_inst;
  }
  // HotSpot (trained on withPrivileges=TRUE):
  // if(withPrivileges) x_inst = SECRET1;
  // else x_inst = 1;
  // if(y_inst > 0) z_inst = 0x12345;
  // else z_inst = 0x23456;
  // ...
}
