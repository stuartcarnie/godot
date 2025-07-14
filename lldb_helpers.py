import lldb
import lldb.formatters.Logger


class VectorViewSynthProvider:
    def __init__(self, valobj, dict):
        logger = lldb.formatters.Logger.Logger()
        self.valobj = valobj
        self.count = None
        self.data = None
        self.data_type = None
        self.data_size = None

    def num_children(self):
        return self.count

    def get_child_index(self, name):
        logger = lldb.formatters.Logger.Logger()
        try:
            return int(name.lstrip("[").rstrip("]"))
        except:
            return -1

    def get_child_at_index(self, index):
        logger = lldb.formatters.Logger.Logger()
        logger >> "Retrieving child " + str(index)
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        
        try:
            offset = index * self.data_size
            # Create a child from the dereferenced pointer
            ptr_value = self.data.GetValueAsUnsigned(0)
            if ptr_value == 0:
                return None
                
            # Create a synthetic child at the correct offset
            child = self.data.CreateValueFromAddress(
                "[" + str(index) + "]",
                ptr_value + offset,
                self.data_type
            )
            return child
        except Exception as e:
            logger >> "Exception creating child: " + str(e)
            return None

    def update(self):
        logger = lldb.formatters.Logger.Logger()
        # preemptively setting this to None - we might end up changing our mind
        # later
        self.count = None
        try:
            # Get the _size member
            size_member = self.valobj.GetChildMemberWithName("_size")
            if not size_member.IsValid():
                logger >> "Failed to get _size member"
                self.count = 0
                return False
                
            self.count = size_member.GetValueAsUnsigned(0)
            
            # Get the _ptr member
            ptr_member = self.valobj.GetChildMemberWithName("_ptr")
            if not ptr_member.IsValid():
                logger >> "Failed to get _ptr member"
                self.count = 0
                return False
            
            # Since _ptr is a pointer, we need to dereference it
            self.data = ptr_member
            self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
            self.data_size = self.data_type.GetByteSize()
            
            # Verify the pointer is valid
            if self.data.GetValueAsUnsigned(0) == 0:
                logger >> "Pointer is null"
                self.count = 0
                return False
                
            return True
        except Exception as e:
            logger >> "Exception in update: " + str(e)
            self.count = 0
        return False

    def has_children(self):
        return self.count > 0


class LocalVectorSynthProvider:
    def __init__(self, valobj, dict):
        logger = lldb.formatters.Logger.Logger()
        self.valobj = valobj
        self.count = None
        self.data = None
        self.data_type = None
        self.data_size = None

    def num_children(self):
        return self.count

    def get_child_index(self, name):
        logger = lldb.formatters.Logger.Logger()
        try:
            return int(name.lstrip("[").rstrip("]"))
        except:
            return -1

    def get_child_at_index(self, index):
        logger = lldb.formatters.Logger.Logger()
        logger >> "Retrieving child " + str(index)
        if index < 0:
            return None
        if index >= self.num_children():
            return None
        offset = index * self.data_size
        return self.data.CreateChildAtOffset(
            "[" + str(index) + "]", offset, self.data_type
        )

    def update(self):
        logger = lldb.formatters.Logger.Logger()
        # preemptively setting this to None - we might end up changing our mind
        # later
        self.count = None
        try:
            self.count = self.valobj.GetChildMemberWithName("count").GetValueAsUnsigned(0)
            self.data = self.valobj.GetChildMemberWithName("data")
            self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
            self.data_size = self.data_type.GetByteSize()
            return True
        except:
            self.count = 0
        return False

    def has_children(self):
        return self.count > 0


class VectorSynthProvider:
    def __init__(self, valobj, dict):
        logger = lldb.formatters.Logger.Logger()
        self.valobj = valobj
        self.uint64_type = valobj.target.GetBasicType(lldb.eBasicTypeUnsignedLongLong)
        self.count = 0
        self.ref_count = None
        self.size = None
        self.data = None
        self.data_type = None
        self.data_size = None
        self.update()

    def num_children(self):
        return self.count + 2 if self.count > 0 else 0

    def get_child_index(self, name):
        if name == "size":
            return self.count + 0
        if name == "ref_count":
            return self.count + 1
        else:
            return int(name.lstrip("[").rstrip("]"))

    def get_child_at_index(self, index):
        if self.count == 0:
            return None
        elif index == self.count + 0:
            return self.size
        elif index == self.count + 1:
            return self.ref_count
        elif index < self.count:
            offset = index * self.data_size
            return self.data.CreateChildAtOffset(
                "[" + str(index) + "]", offset, self.data_type
            )
        else:
            return None

    def update(self):
        self.data = self.valobj.GetChildMemberWithName("_cowdata").GetChildMemberWithName("_ptr")
        self.data_type = self.valobj.GetType().GetTemplateArgumentType(0)
        self.data_size = self.data_type.GetByteSize()

        if self.data.unsigned > 0:
            self.size = self.valobj.CreateValueFromAddress("size", self.data.unsigned - 8, self.uint64_type)
            self.ref_count = self.valobj.CreateValueFromAddress("ref_count", self.data.unsigned - 16, self.uint64_type)
            self.count = self.size.unsigned
        else:
            self.count = 0

    def has_children(self):
        return self.count > 0


def VectorSummaryProvider(valobj, dict):
    return "items=" + str(valobj.num_children - 2 if valobj.num_children > 0 else 0)


def VectorViewSummaryProvider(valobj, dict):
    try:
        num_children = valobj.GetNumChildren()
        return "items=" + str(num_children)
    except:
        return "items=?"


def __lldb_init_module(debugger: lldb.SBDebugger, dict):
    debugger.HandleCommand('type synthetic add -l lldb_helpers.LocalVectorSynthProvider -x "LocalVector<" -w Godot')
    debugger.HandleCommand('type synthetic add -l lldb_helpers.VectorViewSynthProvider -x "VectorView<" -w Godot')
    debugger.HandleCommand('type synthetic add -l lldb_helpers.VectorSynthProvider -x "Vector<.+>$" -w Godot')
    debugger.HandleCommand('type summary add "String" "CharString" --summary-string "${var._cowdata._ptr%s}" --category Godot')
    debugger.HandleCommand('type summary add "Span<char>" --summary-string "${var._ptr%s}" --category Godot')
    debugger.HandleCommand('type summary add "StringName" --summary-string "${var._data->name}" --category Godot')
    debugger.HandleCommand('type summary add "Vector2" "Vector2i" "Point2" "Point2i" --summary-string "\{ x=${var.x}, y=${var.y} \}" --category Godot')
    debugger.HandleCommand('type summary add "Vector3" "Vector3i" --summary-string "\{ x=${var.x}, y=${var.y}, z=${var.z} \}" --category Godot')
    debugger.HandleCommand('type summary add "Vector4" "Vector4i" --summary-string "\{ x=${var.x}, y=${var.y}, z=${var.z}, w=${var.w} \}" --category Godot')
    debugger.HandleCommand('type summary add "Size2" "Size2i" --summary-string "\{ w=${var.width}, h=${var.height} \}" --category Godot')
    debugger.HandleCommand('type summary add "Rect2" "Rect2i" --summary-string "\{ x=${var.position.x}, y=${var.position.height}, w=${var.size.width}, h=${var.size.height} \}" --category Godot')
    debugger.HandleCommand('type summary add "Color" --summary-string "\{ r=${var.r}, g=${var.g}, b=${var.b}, a=${var.a} \}" --category Godot')
    debugger.HandleCommand('type summary add -F lldb_helpers.VectorViewSummaryProvider -x "VectorView<" --expand --category Godot')
    debugger.HandleCommand('type summary add -x "LocalVector<" -x "TightLocalVector<" --expand --summary-string "${svar%#} items" --category Godot')
    debugger.HandleCommand('type summary add -F lldb_helpers.VectorSummaryProvider -x "Vector<.+>$" --expand --category Godot')
    debugger.HandleCommand('type category enable Godot')
